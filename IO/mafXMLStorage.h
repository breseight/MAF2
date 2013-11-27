/*=========================================================================

 Program: MAF2
 Module: mafXMLStorage
 Authors: Marco Petrone
 
 Copyright (c) B3C
 All rights reserved. See Copyright.txt or
 http://www.scsitaly.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#ifndef __mafXMLStorage_h__
#define __mafXMLStorage_h__

#include "mafStorage.h"

//----------------------------------------------------------------------------
// forward declarations :
//----------------------------------------------------------------------------
class mafXMLElement;
class mmuXMLDOM;


/** Concrete implementation of Storage object using Xerces-C for storing in XML.
  This is a concrete implementation of storage object for storing XML documents
  by means of Xerces-C library (http://xml.apache.org/xerces-c/).
  This class also defines a function to access to XML/Xerces-C specific objects, 
  stored into a PIMPL class (mmuXMLDOM).
  @sa mafStorage mafXMLElement mmuXMLDOM
  @todo
    - remove "IncludeWX.h" inclusion from .cpp
    - add support for NULL destination URL
*/  
class MAF_EXPORT mafXMLStorage: public mafStorage
{
public:
  mafTypeMacro(mafXMLStorage,mafStorage);
  
  enum XML_IO_ERRORS {IO_XML_PARSE_ERROR=IO_LAST_ERROR,IO_DOM_XML_ERROR,IO_RESTORE_ERROR,IO_WRONG_FILE_TYPE,IO_WRONG_FILE_VERSION,IO_WRONG_URL,IO_XML_PARSER_INTERNAL_ERROR};

  mafXMLStorage();
  virtual ~mafXMLStorage();

  /** 
    Return the instance of the DOM document used while reading and writing.
    This object is created when Store/Restore starts and destroyed when stops.*/
  mmuXMLDOM *GetXMLDOM() {return m_DOM;}

  /** The TAG identifying the type (i.e. format) of file. (e.g. "MSF") */
  void SetFileType(const char *filetype);
  /** The TAG identifying the type (i.e. format) of file. (e.g. "MSF") */
  const char *GetFileType();

  /** The version of the file format used type of file. (default "1.1") */
  void SetVersion(const char *version);

  /** The version of the file format used type of file. (default "1.1") */
  void SetDictionaryName(const char *dictionaryName);

  /** The version of the file format used type of file. (default "1.1") */
  const char *GetVersion();

  /** Return the version of the opened document.*/
  const char *GetDocumentVersion();

  /** resolve an URL and provide local filename to be used as input */
  virtual int ResolveInputURL(const char * url, mafString &filename, mafObserver *observer = NULL);

  /** resolve an URL and provide a local filename to be used as output */
  virtual int StoreToURL(const char * filename, const char * url);

  /** release file from storage. Actually do not delete, just collect. */
  virtual int ReleaseURL(const char *url);

  /** remove the file from URL */
  virtual int DeleteURL(const char *url);

  /** populate the list of file in the directory */
  virtual int OpenDirectory(const char *pathname);

  /** Set the URL of the document to be read or written */
  virtual void SetURL(const char *name);

  virtual const char* GetTmpFolder();

  /** empty the garbage collector list deleting old files */
  virtual void EmptyGarbageCollector();

  /** get dictionary name */
  const char *GetDictionaryName();

protected:
  /** This is called by Store() and must be reimplemented by subclasses */
  virtual int InternalStore();

  /** This is called by Restore() and must be reimplemented by subclasses */
  virtual int InternalRestore();

  mafString m_DictionaryName;  ///< name of the dictionary
  mafString m_FileType;  ///< The type of file to be opened
  mafString m_Version;   ///< Current MSF version
  mafString m_DocumentVersion; ///< Open Document version.
  mmuXMLDOM *m_DOM;      ///< PIMPL object storing XML objects' pointers
  std::set<mafString> m_GarbageCollector; ///< collect URL to be released
  mafString  m_DefaultTmpFolder; ///< used to store the current default tmp folder
};
#endif // _mafXMLStorage_h_
