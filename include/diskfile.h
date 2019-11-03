// $Id: diskfile.h 2 2019-10-03 18:02:04Z n7dr $

// Released under the GNU Public License, version 2
//   see: https://www.gnu.org/licenses/gpl-2.0.html

// Principal author: N7DR

// Copyright owners:
//    N7DR

/*! \file   diskfile.h

    Useful file-related functions. This file is derived, with permission, from proprietary code
    owned by IPfonix, Inc.
*/

#ifndef DISKFILE_H
#define DISKFILE_H

#include <fstream>
#include <string>
#include <vector>

/*! \brief              Append a string to a file
    \param  filename    name of file
    \param  str         string to be appended

    Creates <i>filename</i> if it does not exist
*/
inline void append_to_file(const std::string& filename, const std::string& str)
  { std::ofstream(filename, std::ios_base::app) << str; }

/*! \brief              Does a file exist?
    \param  filename    name of file
    \return             whether file <i>filename</i> exists

    Actually checks for existence AND readability, which is much simpler
    than checking for existence.
*/
inline const bool file_exists(const std::string& filename)
  { return std::ifstream(filename).good(); }

/*! \brief              What is the size of a file?
    \param  filename    name of file
    \return             length of the file <i>filename</i> in bytes

    Returns <i>false</i> if the file does not exist or it exists and is not readable.
*/
const unsigned long file_size(const std::string& filename);

/*! \brief              Is a file empty?
    \param  filename    name of file
    \return             whether the file <i>filename</i> is empty

    Returns <i>true</i> if the file does not exist or it exists and is not readable.
*/
inline const bool file_empty(const std::string& filename)
  { return (file_size(filename) == 0); }

/*! \brief              Delete a file
    \param  filename    name of file
*/
void file_delete(const std::string& filename);

/*! \brief                          Copy a file
    \param  source_filename         name of the source file
    \param  destination_filename    name of the destination file

    Does nothing if the source does not exist
*/
void file_copy(const std::string& source_filename, const std::string& destination_filename);

/*! \brief                          Rename a file
    \param  source_filename         original name of file
    \param  destination_filename    final name of file

    Does nothing if the source file does not exist
*/
void file_rename(const std::string& source_filename, const std::string& destination_filename);

/*! \brief                          Move a file
    \param  source_filename         original filename
    \param  destination_filename    final filename

    Does nothing if the source file does not exist
*/
inline void file_move(const std::string& source_filename, const std::string& destination_filename)
  { file_rename(source_filename, destination_filename); }

/*! \brief              Create a directory
    \param  dirname     name of the directory to create

    Throws exception if the directory already exists
*/
void directory_create(const std::string& dirname);

/*! \brief              Does a directory exist?
    \param  dirname     name of the directory to test for existence
    \return             whether <i>dirname</i> exists
*/
const bool directory_exists(const std::string& dirname);

/*! \brief              Create a directory
    \param  dirname     name of the directory to create

    Directory is created only if it does not already exist
*/
void directory_create_if_necessary(const std::string& dirname);

/*! \brief              What files (including directories) does a directory contain?
    \param  dirname     name of the directory to examine
    \return             vector of filenames

    The returned vector does not include "." or "..".
    Returns empty vector if the directory <i>dirname</i> does not exist
*/
const std::vector<std::string> directory_contents(const std::string& dirname);

/*! \brief              What files (links or regular files) does a directory contain?
    \param  dirname     name of the directory to examine
    \return             vector of filenames

    Returns empty vector if the directory <i>dirname</i> does not exist
*/
const std::vector<std::string> files_in_directory(const std::string& dirname);

/*! \brief              Truncate a file
    \param  filename    name of file to truncate

    Creates <i>filename</i> if it does not exist
*/
inline void file_truncate(const std::string& filename)
  { std::ofstream(filename, std::ios_base::trunc); }
  
/*! \brief              Get the full directory path from a filename
    \param  filename    name of file to truncate
    \return             the directory in which <i>filename</i> is located
    
    The returned value ends in a slash.
*/
const std::string directory_name(const std::string& filename);

#endif    // DISKFILE_H
