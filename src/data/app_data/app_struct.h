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
 * An AppStruct is an application object corresponding to a set of nimbus
 * variables, provided by application managers.
 *
 * Author: Chinmayee Shah <chshah@stanford.edu>
 */

#ifndef NIMBUS_SRC_DATA_APP_DATA_APP_STRUCT_H_
#define NIMBUS_SRC_DATA_APP_DATA_APP_STRUCT_H_

#include <map>
#include <set>
#include <string>
#include <vector>

#include "src/data/app_data/app_data_defs.h"
#include "src/data/app_data/app_object.h"
#include "src/shared/nimbus_types.h"

namespace nimbus {

class Data;
typedef std::vector<Data *> DataArray;
typedef std::set<Data *> DataSet;
class GeometricRegion;

/**
 * \class AppStruct
 * \details Application object corresponding to a set of nimbus variables
 * provided by app data managers. A AppStruct can have num_variables number of
 * different nimbus variables, and must be defined over a geometric region.
 * Read and write calls contain a type, corresponding to each read/ write set/
 * data. This type is an integer, between zero (inclusive) and num_variables_
 * (exclusive).
 */
class AppStruct : public AppObject {
    friend class AppDataManager;
    friend class CacheManager;
    friend class VDataCacheManager;
    friend class SimpleAppDataManager;
    friend class CacheTable;
    public:
        /**
         * \brief Creates an AppStruct
         * \param num_variables indicates number of different application
         * variables that the AppStruct instance contains
         * \return Constructed AppStruct instance
         */
        explicit AppStruct(size_t num_variables);

        /**
         * \brief Creates an AppStruct
         * \param num_variables indicates number of different application
         * variables that the AppStruct instance contains
         * \param  ob_reg specifies application object (AppStruct) region
         * \return Constructed AppStruct instance
         */
        explicit AppStruct(size_t num_variables, const GeometricRegion &ob_reg);

    private:
        // number of nimbus variables
        size_t num_variables_;

        // app-data mappings
        typedef std::map<GeometricRegion,
                         Data *> DMap;
        std::vector<DMap> data_maps_;
        std::vector<DataSet> write_backs_;

        // disable constructor without arguments
        AppStruct() {}

        // methods for app manager to manage mappings and control access
        void SetUpReadWrite(const std::vector<app_data::type_id_t> &var_type,
                            const std::vector<DataArray> &read_sets,
                            const std::vector<DataArray> &write_sets,
                            std::vector<DataArray> *flush_sets,
                            std::vector<DataArray> *diff_sets,
                            std::vector<DataArray> *sync_sets,
                            std::vector<AppObjects> *sync_ob_sets);

        bool CheckPendingFlag(const std::vector<app_data::type_id_t> &var_type,
                              const std::vector<DataArray> &read_sets,
                              const std::vector<DataArray> &write_sets);

        void ReleasePendingFlag(const std::vector<app_data::type_id_t> &var_type,
                                std::vector<DataArray> *flush_sets,
                                std::vector<DataArray> *diff_sets,
                                std::vector<DataArray> *sync_sets);

        app_data::distance_t GetDistance(const std::vector<app_data::type_id_t> &var_type,
                                      const std::vector<DataArray> &read_sets) const;

    protected:
        /**
         * \brief Reads data from read_sets into AppStruct instance
         * \param var_type is a list of type_ids corresponding to nimbus variables,
         * as explained in the class description
         * \param read_sets is a list of data arrays corresponding to nimbus variables
         * \param read_region is the geometric region to read
         * \details This function must be overwritten by the application
         * writer. It provides the transformation from a set of nimbus data to
         * application instance.
         */
        virtual void ReadAppData(const std::vector<app_data::type_id_t> &var_type,
                                 const std::vector<DataArray> &read_sets,
                                 const GeometricRegion &read_region) = 0;

        /**
         * \brief Writes data from AppStruct instance to write_sets
         * \param var_type is a list of type_ids corresponding to nimbus variables,
         * as explained in the class description
         * \param write_sets is a list of data arrays corresponding to nimbus variables
         * \param write_region is the geometric region to be write
         * \details This function must be overwritten by the application
         * writer. It provides the transformation from application instance to
         * nimbus data.
         */
        virtual void WriteAppData(const std::vector<app_data::type_id_t> &var_type,
                                  const std::vector<DataArray> &write_sets,
                                  const GeometricRegion &write_region) const = 0;

        /**
         * \brief Creates a new AppStruct instance using current instance
         * parameters
         * \param struct_region specifies the spatial domain of the AppStruct
         * instance
         * \return Returns a pointer to the newly allocated AppStruct instance
         * \details This is a virtual function that must be over-written by application
         * writer. When AppManager cannot satisfy an application object request,
         * using the instances it already has, it calls CreateNew(..) on the
         * prototype passed in the request.
         */
        virtual AppStruct *CreateNew(const GeometricRegion &struct_region) const = 0;

        /**
         * \brief Frees allocated memory. Application writer must implement
         * this.
         */
        virtual void Destroy() = 0;

    protected:
        /**
         * \brief Pulls data from app, removes corresponding dirty data
         * mapping. Locks the app object when pulling the data.
         * \param d is data to flush to
         */
        virtual void PullData(Data *d);

        /**
         * \brief Unsets mapping between data and AppStruct instance
         * \param d denotes the data to unmap
         */
        virtual void UnsetData(Data *d);

        /**
         * \brief Unsets dirty data mapping between data and AppStruct
         * instance
         * \param d denotes the data to unmap
         */
        virtual void UnsetDirtyData(Data *d);

        // method for app manager for profiling
        virtual size_t memory_size() {
          return sizeof(*this);
        }
};  // class AppStruct

typedef std::vector<AppStruct *> AppStructs;

}  // namespace nimbus

#endif  // NIMBUS_SRC_DATA_APP_DATA_APP_STRUCT_H_
