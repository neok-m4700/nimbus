/*
 * Copyright 2013 Stanford University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of the copyright holders nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * CacheManager implements application data manager with simple_app_data caching of
 * application data across jobs.
 *
 * Author: Chinmayee Shah <chshah@stanford.edu>
 */

#include <sys/syscall.h>
#include <pthread.h>
#include <cstdio>
#include <ctime>
#include <map>
#include <vector>
#include <string>

#include "src/data/app_data/app_data_defs.h"
#include "src/data/app_data/app_object.h"
#include "src/shared/dbg.h"
#include "src/shared/fast_log.h"
#include "src/shared/geometric_region.h"
#include "src/worker/app_data_manager.h"
#include "src/worker/app_data_managers/cache_manager.h"
#include "src/worker/app_data_managers/cache_table.h"
#include "src/worker/data.h"

#define FIRST_UNIQUE_ID 1000

namespace nimbus {
/**
 * \details
 */
CacheManager::CacheManager() {
    unique_id_allocator_ = FIRST_UNIQUE_ID;
    pool_ = new Pool();
    pthread_mutex_init(&cache_lock, NULL);
    pthread_cond_init(&cache_cond, NULL);
}

CacheManager::~CacheManager() {}

/**
 * \details CacheManager writes back all the dirty data from write_sets back to
 * nimbus objects. Note that this is not thread safe, this function does not
 * use locks or pending flags. This is guaranteed to work correctly only under
 * the assumption:
 * The mapping for the cached object and corresponding nimbus objects (dirty)
 * should not be edited simultaneously by other thread. This will hold for a
 * thread if the write_back set is a subset of write set for the job.
 * Locking should probably not add much overhead, but it is not necessary right
 * now.
 */
void CacheManager::WriteImmediately(AppVar *app_var,
                                    const DataArray &write_set) {
    DataArray flush_set;
    DataSet &write_back = app_var->write_back_;
    for (size_t i = 0; i < write_set.size(); ++i) {
        Data *d = write_set[i];
        if (write_back.find(d) != write_back.end()) {
            flush_set.push_back(d);
        }
    }
    for (size_t i = 0; i < flush_set.size(); ++i) {
        Data *d = flush_set[i];
        d->UnsetDirtyAppObject(app_var);
        write_back.erase(d);
    }

    nimbus::timer::StartTimer(nimbus::timer::kWriteAppData);
    app_var->WriteAppData(flush_set, app_var->write_region_);
    nimbus::timer::StopTimer(nimbus::timer::kWriteAppData);
}

/**
 * \details CacheManager writes back all the dirty data from write_sets back to
 * nimbus objects. Note that this is not thread safe, this function does not
 * use locks or pending flags. This is guaranteed to work correctly only under
 * the assumption:
 * The mapping for the cache object and corresponding nimbus objects (dirty) is
 * not edited simultaneously by other thread. This will hold for a thread if
 * the write_back set is a subset of write set for the job.
 * Locking should probably not add much overhead, but it is not necessary at
 * right now.
 */
void CacheManager::WriteImmediately(AppStruct *app_struct,
                                    const std::vector<app_data::type_id_t> &var_type,
                                    const std::vector<DataArray> &write_sets) {
    size_t num_vars = var_type.size();
    if (write_sets.size() != num_vars) {
        dbg(DBG_ERROR, "Mismatch in number of variable types passed to FlushCache\n");
        exit(-1);
    }
    std::vector<DataArray> flush_sets(num_vars);
    std::vector<DataSet> &write_backs = app_struct->write_backs_;
    for (size_t t = 0; t < num_vars; ++t) {
        DataArray &flush_t = flush_sets[t];
        const DataArray &write_set_t = write_sets[t];
        app_data::type_id_t type = var_type[t];
        DataSet &write_back_t = write_backs[type];
        for (size_t i = 0; i < write_set_t.size(); ++i) {
            Data *d = write_set_t[i];
            if (write_back_t.find(d) != write_back_t.end()) {
                flush_t.push_back(d);
            }
        }
    }
    for (size_t t = 0; t < num_vars; ++t) {
        const DataArray &flush_t = flush_sets[t];
        app_data::type_id_t type = var_type[t];
        DataSet &write_back_t = write_backs[type];
        for (size_t i = 0; i < flush_t.size(); ++i) {
            Data *d = flush_t[i];
            d->UnsetDirtyAppObject(app_struct);
            write_back_t.erase(d);
        }
    }


    nimbus::timer::StartTimer(nimbus::timer::kWriteAppData);
    app_struct->WriteAppData(var_type, flush_sets,
                             app_struct->write_region_);
    nimbus::timer::StopTimer(nimbus::timer::kWriteAppData);
}

/**
 * \details CacheManager checks if an instance with requested application
 * object region and prototype id is present and available for use. If not, it
 * creates a new instance, and adds the instance to its 2-level map. In both
 * cases, it acquires the object, assembles it (mappings, read diff, flush data
 * to be overwritten etc) and then returns.
 * The access mode is overwritten as EXCLUSIVE right now, meaning 2 parallel
 * compute jobs cannot operate on the same cache object.
 */
// TODO(hang/chinmayee): remove *aux, aux_data.
AppVar *CacheManager::GetAppVarV(const DataArray &read_set,
                                 const GeometricRegion &read_region,
                                 const DataArray &write_set,
                                 const GeometricRegion &write_region,
                                 const AppVar &prototype,
                                 const GeometricRegion &region,
                                 app_data::Access access,
                                 void (*aux)(AppVar*, void*),
                                 void* aux_data) {
    // TODO(chinmayee): Remove this when application objects can be shared by compute jobs
    access = app_data::EXCLUSIVE;
    pthread_mutex_lock(&cache_lock);
    AppVar *cv = NULL;
    // Get a cache object form the cache table.
    if (pool_->find(prototype.id()) == pool_->end()) {
        CacheTable *ct = new CacheTable(app_data::VAR);
        (*pool_)[prototype.id()] = ct;
        cv = prototype.CreateNew(region);
        cv->set_unique_id(unique_id_allocator_++);
        assert(cv != NULL);
        ct->AddEntry(region, cv);
    } else {
        CacheTable *ct = (*pool_)[prototype.id()];
        cv = ct->GetClosestAvailable(region, read_set, access);
        if (cv == NULL) {
            cv = prototype.CreateNew(region);
            cv->set_unique_id(unique_id_allocator_++);
            assert(cv != NULL);
            ct->AddEntry(region, cv);
        }
    }
    cv->AcquireAccess(access);
    DataArray flush, sync, diff;
    AppObjects sync_co;
    while (!cv->CheckPendingFlag(read_set, write_set)) {
      pthread_cond_wait(&cache_cond, &cache_lock);
    }
    cv->SetUpReadWrite(read_set, write_set,
                       &flush, &diff, &sync, &sync_co);
    pthread_mutex_unlock(&cache_lock);
    if (aux != NULL) {
      aux(cv, aux_data);
    }

    GeometricRegion write_region_old = cv->write_region_;
    cv->write_region_ = write_region;

    nimbus::timer::StartTimer(nimbus::timer::kWriteAppData);
    cv->WriteAppData(flush, write_region_old);
    nimbus::timer::StopTimer(nimbus::timer::kWriteAppData);

    for (size_t i = 0; i < sync.size(); ++i) {
        sync_co[i]->PullData(sync[i]);
    }

    pthread_mutex_lock(&cache_lock);
    cv->ReleasePendingFlag(&flush, &diff, &sync);
    pthread_cond_broadcast(&cache_cond);
    pthread_mutex_unlock(&cache_lock);

    nimbus::timer::StartTimer(nimbus::timer::kReadAppData);
    cv->ReadAppData(diff, read_region);
    nimbus::timer::StopTimer(nimbus::timer::kReadAppData);

    return cv;
}

/**
 * \details CacheManager checks if an instance with requested application
 * object region and prototype id is present and available for use. If not, it
 * creates a new instance, and adds the instance to its 2-level map. In both
 * cases, it acquires the object, assembles it (mappings, read diff, flush data
 * to be overwritten etc) and then returns.
 * The access mode is overwritten as EXCLUSIVE right now, meaning 2 parallel
 * compute jobs cannot operate on the same cache object.
 */
AppStruct *CacheManager::GetAppStructV(const std::vector<app_data::type_id_t> &var_type,
                                       const std::vector<DataArray> &read_sets,
                                       const GeometricRegion &read_region,
                                       const std::vector<DataArray> &write_sets,
                                       const GeometricRegion &write_region,
                                       const AppStruct &prototype,
                                       const GeometricRegion &region,
                                       app_data::Access access) {
    // TODO(chinmayee): Remove this when application objects can be shared by compute jobs
    access = app_data::EXCLUSIVE;
    pthread_mutex_lock(&cache_lock);
    AppStruct *cs = NULL;
    if (pool_->find(prototype.id()) == pool_->end()) {
        CacheTable *ct = new CacheTable(app_data::STRUCT);
        (*pool_)[prototype.id()] = ct;
        cs = prototype.CreateNew(region);
        cs->set_unique_id(unique_id_allocator_++);
        assert(cs != NULL);
        ct->AddEntry(region, cs);
    } else {
        CacheTable *ct = (*pool_)[prototype.id()];
        cs = ct->GetClosestAvailable(region, var_type, read_sets, access);
        if (cs == NULL) {
            cs = prototype.CreateNew(region);
            cs->set_unique_id(unique_id_allocator_++);
            assert(cs != NULL);
            ct->AddEntry(region, cs);
        }
    }
    size_t num_var = var_type.size();
    std::vector<DataArray> flush_sets(num_var),
                           sync_sets(num_var),
                           diff_sets(num_var);
    std::vector<AppObjects> sync_co_sets(num_var);
    // Move here.
    cs->AcquireAccess(access);
    while (!cs->CheckPendingFlag(var_type, read_sets, write_sets)) {
      pthread_cond_wait(&cache_cond, &cache_lock);
    }
    cs->SetUpReadWrite(var_type, read_sets, write_sets,
                       &flush_sets, &diff_sets, &sync_sets, &sync_co_sets);
    pthread_mutex_unlock(&cache_lock);

    GeometricRegion write_region_old = cs->write_region_;
    cs->write_region_ = write_region;

    nimbus::timer::StartTimer(nimbus::timer::kWriteAppData);
    cs->WriteAppData(var_type, flush_sets, write_region_old);
    nimbus::timer::StopTimer(nimbus::timer::kWriteAppData);

    for (size_t t = 0; t < num_var; ++t) {
        DataArray &sync_t = sync_sets[t];
        AppObjects &sync_co_t = sync_co_sets[t];
        for (size_t i = 0; i < sync_t.size(); ++i) {
            sync_co_t[i]->PullData(sync_t[i]);
        }
    }

    pthread_mutex_lock(&cache_lock);
    cs->ReleasePendingFlag(var_type,
                           &flush_sets, &diff_sets, &sync_sets);
    pthread_cond_broadcast(&cache_cond);
    pthread_mutex_unlock(&cache_lock);

    nimbus::timer::StartTimer(nimbus::timer::kReadAppData);
    cs->ReadAppData(var_type, diff_sets, read_region);
    nimbus::timer::StopTimer(nimbus::timer::kReadAppData);

    return cs;
}

/**
 * \details Pulls data from dirty app object (if there is one), updates
 * mappings and returns.
 */
void CacheManager::SyncData(Data *d) {
    AppObject *co = NULL;
    pthread_mutex_lock(&cache_lock);
    while (d->pending_flag() != 0) {
       pthread_cond_wait(&cache_cond, &cache_lock);
    }
    co = d->dirty_app_object();
    if (!co) {
        pthread_mutex_unlock(&cache_lock);
        return;
    }
    // assert(co->IsAvailable(app_data::EXCLUSIVE));
    d->set_pending_flag(Data::WRITE);
    pthread_mutex_unlock(&cache_lock);

    co->PullData(d);
    pthread_mutex_lock(&cache_lock);
    d->ClearDirtyMappings();
    d->unset_pending_flag(Data::WRITE);
    pthread_cond_broadcast(&cache_cond);
    pthread_mutex_unlock(&cache_lock);
}

/**
 * \details Removes mappings between this data object and any corresponding
 * cached application objects.
 */
void CacheManager::InvalidateMappings(Data *d) {
    pthread_mutex_lock(&cache_lock);
    while (d->pending_flag() != 0) {
        pthread_cond_wait(&cache_cond, &cache_lock);
    }
    d->InvalidateMappings();
    pthread_mutex_unlock(&cache_lock);
}

void CacheManager::ReleaseAccess(AppObject* app_object) {
    pthread_mutex_lock(&cache_lock);
    app_object->ReleaseAccessInternal();
    pthread_cond_broadcast(&cache_cond);
    pthread_mutex_unlock(&cache_lock);
}

}  // namespace nimbus
