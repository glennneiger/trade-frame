#pragma once

//#include "BerkeleyDb.h"
#include <db_cxx.h>

#include "H5Cpp.h"

#include <string>

// manages Berkeley database and HDF5 Database
class CDataManager {
public:
  CDataManager(void);
  virtual ~CDataManager(void);
  DbEnv *GetDbEnv( void ) { return &m_DbEnv; }; 
  const char *GetBDBFileName( void ) { return m_BDBFileName; };
  H5::H5File *GetH5File( void ) { return &m_H5File; };
  void AddGroup( const std::string &sGroup );
  void AddGroupForSymbol( const std::string &sSymbol );
  static herr_t PrintH5ErrorStackItem( int n, H5E_error_t *err_desc, void *client_data );
  static hsize_t H5ChunkSize( void ) { return 64; };
protected:
  static const char m_H5FileName[];
  static const char m_BDBFileName[];
  static unsigned int m_RefCount;
  static DbEnv m_DbEnv;
  static H5::H5File m_H5File;
private:
};
