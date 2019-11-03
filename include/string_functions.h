// $Id: string_functions.h 15 2019-11-03 15:02:05Z n7dr $

// Released under the GNU Public License, version 2
//   see: https://www.gnu.org/licenses/gpl-2.0.html

// Principal author: N7DR

// Copyright owners:
//    N7DR

/*! \file   string_functions.h

    Functions related to the manipulation of strings
*/

#ifndef STRING_FUNCTIONS_H
#define STRING_FUNCTIONS_H

#include "macros.h"
#include "x_error.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <deque>
#include <iomanip>
#include <sstream>
#include <string>
#include <experimental/string_view>
#include <type_traits>
#include <vector>

#include <time.h>

using namespace std::literals::string_literals;
using namespace std::experimental::literals::string_view_literals;

/*
 https://stackoverflow.com/questions/459942/defining-class-string-constants-in-c:

Usually you should prefer std::string over plain char pointers. Here, however, the char pointer initialized
with the string literal has a significant benefit.

There are two initializations for static data. The one is called static initialization, and the other is called
dynamic initialization. For those objects that are initialized with constant expressions and that are PODs
(like pointers), C++ requires that their initialization happens at the very start, before dynamic initialization
happens. Initializing such an std::string will be done dynamically.

If you have an object of a class being a static object in some file, and that one needs to access the string during
its initialization, you can rely on it being set-up already when you use the const char* const version, while using
the std::string version, which isn't initialized statically, you don't know whether the string is already initialized -
because the order of initialization of objects across translation unit boundaries is not defined.
*/

#define CONST_STRING_TYPE const char* const

CONST_STRING_TYPE EOL      = "\n";       ///< end-of-line marker as string
static const char        EOL_CHAR = '\n';       ///< end-of-line marker as character

CONST_STRING_TYPE  LF       = "\n";      ///< LF as string
//CONST_STRING_TYPE& LF_STR   = LF;        ///< LF as string
static const char         LF_CHAR  = '\n';      ///< LF as character

CONST_STRING_TYPE CR       = "\r";       ///< CR as string
static const char        CR_CHAR  = '\r';       ///< CR as character
CONST_STRING_TYPE CRLF     = "\r\n";     ///< CR followed by LF

CONST_STRING_TYPE EMPTY_STR = "";
CONST_STRING_TYPE SPACE_STR = " ";       ///< space as string

CONST_STRING_TYPE CALLSIGN_CHARS { "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ/" };                ///< convenient place to hold all characters that are legal in callsigns
CONST_STRING_TYPE DIGITS { "0123456789" };                                                   ///< convenient place to hold all digits
CONST_STRING_TYPE DIGITS_AND_UPPER_CASE_LETTERS { "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ" };  ///< convenient place to hold all digits and upper case letters
//static const std::string UPPER_CASE_LETTERS { "ABCDEFGHIJKLMNOPQRSTUVWXYZ" };                       ///< convenient place to hold all upper case letters
CONST_STRING_TYPE UPPER_CASE_LETTERS { "ABCDEFGHIJKLMNOPQRSTUVWXYZ" };                       ///< convenient place to hold all upper case letters

constexpr char SPACE_CHAR { ' ' };

//const std::string BURBLE_STRING = "THIS IS A BURBLE STRING DEFINED IN THE HEADER";
//extern const std::string BURBLE_STRING;

constexpr bool INCLUDE_SECONDS { true };             ///< whether to include seconds in date_time_string()
  
/// directions in which a string can be padded
enum pad_direction { PAD_LEFT,                  ///< pad to the left
                     PAD_RIGHT                  ///< pad to the right
                   };

// error numbers
constexpr int STRING_UNDERFLOW            { -1 },    ///< Underflow
              STRING_UNEXPECTED_CHARACTER { -2 },    ///< Unexpected character found in string
              STRING_INVALID_FILE         { -3 },    ///< Invalid file
              STRING_INVALID_LENGTH       { -4 },    ///< Length is invalid
              STRING_FILE_IS_DIRECTORY    { -5 },    ///< File is a directory
              STRING_UNABLE_TO_STAT_FILE  { -6 },    ///< Unable to stat a file
              STRING_BOUNDS_ERROR         { -7 },    ///< Attempt to access range outside string
              STRING_CONVERSION_FAILURE   { -8 },    ///< Attempt to convert the format of a string failed
              STRING_UNKNOWN_ENCODING     { -9 },    ///< Unknown character encoding
              STRING_UNWRITEABLE_FILE     { -10 };   ///< File cannot be written

/*! \brief          Return position in a string at the end of a target string, if present
    \param  str     string to search
    \param  target  string to find
    \return         position in <>str</i> after the end of <i>target</i>

    Returns string::npos if <i>target</i> is not a substring of <i>str</i> OR if <i>target</i>
    is the conclusion of <i>str</i>
*/
//const size_t find_and_go_to_end_of(const std::string& str, const std::string& target);
const size_t find_and_go_to_end_of(const std::experimental::string_view str, const std::experimental::string_view target);

/*! \brief          Convert from a CSV line to a vector of strings, each containing one field
    \param  line    CSV line
    \return         vector of fields from the CSV line

    This is actually quite difficult to do properly
*/
const std::vector<std::string> from_csv(const std::experimental::string_view line);

/*! \brief      Duplicate a particular character within a string
    \param  s   string in which characters are to be duplicated
    \param  c   character to be duplicated
    \return     <i>s</i>, modified so that every instance of <i>c</i> is doubled
*/
const std::string duplicate_char(const std::experimental::string_view s, const char c = '"');

/*! \brief                      Provide a formatted date/time string
    \param  include_seconds     whether to include the portion of the string that designates seconds
    \return                     current date and time in the format: YYYY-MM-DDTHH:MM or YYYY-MM-DDTHH:MM:SS
*/
const std::string date_time_string(const bool include_seconds = !INCLUDE_SECONDS);

/*! \brief          Convert struct tm pointer to formatted string
    \param  format  format to be used
    \param  tmp     date/time to be formatted
    \return         formatted string

    Uses strftime() to perform the formatting
*/
const std::string format_time(const std::string& format, const tm* tmp);

/*! \brief      Generic conversion from string
    \param  s   string
    \return     <i>s</i> converted to type <i>T</i>
*/
template <class T>
const T from_string(const std::string& s)
{ std::istringstream stream { s };
  T t;
     
  stream >> t;
  return t;
}

/*! \brief      Generic conversion from string
    \param  s   string
    \return     <i>s</i> converted to type <i>T</i>

    https://stackoverflow.com/questions/41009325/how-to-make-istringstream-more-efficient:
      I'm not sure is this is the reason, but I suspect this isn't supported because the internal
      storage is backed by stringbuf, which doesn't accept string_view (for good reason).
*/
template <class T>
const T from_string(const std::experimental::string_view s)
{ std::istringstream stream;
  T t;

  stream.rdbuf()->pubsetbuf(const_cast<char*>(s.data()), s.size());

  stream >> t;
  return t;
}

/*! \brief          Generic conversion to string
    \param  val     value to convert
    \return         <i>val</i>converted to a string
*/
template <class T>
const std::string to_string(const T val)
{ std::ostringstream stream;
                
  stream << val;
  return stream.str();
}

/*! \brief          Generic conversion of a REAL type to string with particular number of digits after the decimal point
    \param  val     value to convert
    \param  dp      number of digits after the decimal point
    \return         <i>val</i>converted to a string
*/
template <class T> requires std::is_floating_point<T>::value 
const std::string to_string(const T val, const int dp)
{ std::ostringstream stream;
                
  stream << std::fixed << std::setprecision(dp) << val;
  return stream.str();
}

/*! \brief              Safe version of the substr() member function
    \param  str         string on which to operate
    \param  start_posn  position at which to start operation
    \param  length      length of substring to be extracted
    \return             substring of length <i>length</i>, starting at position <i>start_posn</i>

    Operates like <i>str.substr(start_posn, length)</i>, except does not throw a range exception
    Returns empty string if there is a problem.
*/
inline const std::string substring(const std::experimental::string_view str, const size_t start_posn, const size_t length)
  { return ( (str.size() > start_posn) ? std::string(str.substr(start_posn, length)) : std::string() ); }

/*! \brief              Safe version of the substr() member function
    \param  str         string on which to operate
    \param  start_posn  position at which to start operation
    \return             substring starting at position <i>start_posn</i>

    Operates like <i>str.substr(start_posn)</i>, except does not throw a range exception
    Returns empty string if there is a problem.
*/
inline const std::string substring(const std::experimental::string_view str, const size_t start_posn)
  { return ( (str.size() > start_posn) ? std::string(str.substr(start_posn)) : std::string() ); }

/*! \brief              Replace every instance of one character with another
    \param  s           string on which to operate
    \param  old_char    character to be replaced
    \param  new_char    replacement character
    \return             <i>s</i>, with every instance of <i>old_char</i> replaced by <i>new_char</i>
*/
const std::string replace_char(const std::experimental::string_view s, const char old_char, const char new_char);

/*! \brief              Replace every instance of one string with another
    \param  s           string on which to operate
    \param  old_str     string to be replaced
    \param  new_str     replacement string
    \return             <i>s</i>, with every instance of <i>old_str</i> replaced by <i>new_str</i>
*/
const std::string replace(const std::string& s, const std::string& old_str, const std::string& new_str);

/*! \brief              Replace part of a string with a byte-for-byte copy of an object
    \param  s           string on which to operate
    \param  start_posn  position at substitution begins
    \param  value       bytes that are to be placed into <i>s</i>
    \return             <i>s</i>, overwriting characters starting at <i>start_posn</i> with the bytes of <i>value</i>

    Will not return a string of length greater than <i>s</i>; will truncate to that length if necessary
*/
template <typename T>
const std::string replace_substring(const std::string& s, const size_t start_posn, const T& value)
{ std::string rv { s };

  constexpr size_t value_size { sizeof(value) };

  u_char* cp { (u_char*)&value };

  for (size_t n = 0; n < value_size; ++n)
  { if ( (start_posn + n) < rv.size())
      rv[start_posn + n] = cp[n];
  }

  return rv;
}

/*! \brief      Does a string contain a particular substring?
    \param  s   string to test
    \param  ss  substring for which to search
    \return     whether <i>s</i> contains the substring <i>ss</i>
*/
//inline const bool contains(const std::string& s, const std::string& ss)
inline const bool contains(std::experimental::string_view s, std::experimental::string_view ss)
  { return s.find(ss) != std::string::npos; }

/*! \brief      Does a string contain a particular character?
    \param  s   string to test
    \param  c   character for which to search
    \return     whether <i>s</i> contains the character <i>c</i>
*/
inline const bool contains(const std::string& s, const char c)
  { return s.find(c) != std::string::npos; }

/*! \brief          Does a string contain any letters?
    \param  str     string to test
    \return         whether <i>str</i> contains any letters
*/
const bool contains_letter(const std::string& str);

/*! \brief          Does a string contain any upper case letters?
    \param  str     string to test
    \return         whether <i>str</i> contains any upper case letters
*/
inline const bool contains_upper_case_letter(const std::string& str)
  { return (str.find_first_of(UPPER_CASE_LETTERS) != std::string::npos); }

/*! \brief          Does a string contain any digits?
    \param  str     string to test
    \return         whether <i>str</i> contains any digits
*/
const bool contains_digit(const std::string& str);

/*! \brief              Pad a string to a particular size
    \param  s           original string
    \param  len         length of returned string
    \param  pad_side    side on which to pad
    \param  pad_char    character with which to pad
    \return             padded version of <i>s</i>
  
    If <i>s</i> is already longer than <i>len</i>, then <i>s</i> is returned.
*/
//const std::string pad_string(const std::string& s, const unsigned int len, const enum pad_direction pad_side = PAD_LEFT, const char pad_char = ' ');
//const std::string pad_string(std::experimental::string_view s, const unsigned int len, const enum pad_direction pad_side = PAD_LEFT, const char pad_char = ' ');
//const std::string pad_string(std::experimental::string_view s, const size_t len, const enum pad_direction pad_side = PAD_LEFT, const char pad_char = ' ');

/*! \brief  Create a string of a certain length, with all characters the same
    \param  c   Character that the string will contain
    \param  n   Length of string to be created
    \return String containing <i>n</i> copies of <i>c</i>
*/
inline const std::string create_string(const char c, const int n = 1)
  { return std::string(n, c); }

/*! \brief          Centre a string
    \param  str     string to be centred
    \param  width   final width of the centred string
    \return         <i>str</i> centred in a string of spaces, with total size <i>width</i>,
*/
const std::string create_centred_string(const std::string& str, const unsigned int width);

template <typename T>
const std::string pad_string(T&& s, const size_t len, const enum pad_direction pad_side = PAD_LEFT, const char pad_char = ' ')
{ std::string rv { to_string(s) };

  if (rv.length() >= len)
    return rv;

  const size_t      n_pad_chars { (len - rv.length()) };
  const std::string pad_string  { create_string(pad_char, n_pad_chars) };

  if (pad_side == PAD_LEFT)
    rv = pad_string + rv;
  else
    rv += pad_string;

  return rv;
}

//inline const std::string pad_string(const std::string& s, const unsigned int len, const enum pad_direction pad_side = PAD_LEFT, const char pad_char = ' ')
//  { return pad_string( static_cast<std::experimental::string_view>(s), len, pad_side, pad_char ); }

/*! \brief              Read the contents of a file into a single string
    \param  filename    name of file to be read
    \return             contents of file <i>filename</i>
  
    Throws exception if the file does not exist, or if any
    of several bad things happen. Assumes that the file is a reasonable length.
*/
const std::string read_file(const std::string& filename);

/*! \brief              Read the contents of a file into a single string
    \param  path        the different directories to try, in order
    \param  filename    name of file to be read
    \return             contents of file <i>filename</i>

    Throws exception if the file does not exist, or if any
    of several bad things happen. Assumes that the file is a reasonable length.
*/
const std::string read_file(const std::vector<std::string>& path, const std::string& filename);

/*! \brief              Read the contents of a file into a single string
    \param  filename    name of file to be read
    \param  path        the different directories to try, in order
    \return             contents of file <i>filename</i>

    Throws exception if the file does not exist, or if any
    of several bad things happen. Assumes that the file is a reasonable length.
*/
inline const std::string read_file(const std::string& filename, const std::vector<std::string>& path)
  { return read_file(path, filename); }

/*! \brief              Write a string to a file
    \param  cs          string to be written to file
    \param  filename    name of file to be written

    Throws exception if the file cannot be written
*/
void write_file(const std::string& cs, const std::string& filename);

/*! \brief      Does a string begin with a particular substring?
    \param  cs  string to test
    \param  ss  substring to look for
    \return     whether <i>cs</i> begins with <i>ss</i>
*/
inline const bool starts_with(std::experimental::string_view cs, std::experimental::string_view ss)
  { return (cs.find(ss) == 0); }

/*! \brief      Does a string begin with a particular substring?
    \param  cs  string to test
    \param  ss  substring to look for
    \return     whether <i>cs</i> begins with <i>ss</i>
*/
inline const bool begins_with(std::experimental::string_view cs, std::experimental::string_view ss)
  { return (starts_with(cs, ss) ); }

/*! \brief      Does a string end with a particular substring?
    \param  cs  string to test
    \param  ss  substring to look for
    \return     whether <i>cs</i> ends with <i>ss</i>
*/
inline const bool ends_with(std::experimental::string_view cs, std::experimental::string_view ss)
  { return ( (cs.length() < ss.length()) ? false : ( cs.rfind(ss) == (cs.length() - ss.length()) ) ); }

/*! \brief      Remove characters from the end of a string
    \param  s   original string
    \param  n   number of chars to remove
    \return     <i>s</i> with the last <i>n</i> characters removed
  
    If <i>n</i> is equal to or greater than the length of <i>s</i>, then
    the empty string is returned.
*/
inline const std::string remove_from_end(const std::string& s, const unsigned int n)
  { return ( (n >= s.length()) ? std::string() : s.substr(0, s.length() - n) ); }

/*! \brief      Remove characters if present at the end of a string
    \param  s   original string
    \param  e   string to remove
    \return     <i>s</i> with the <i>e</i> removed, if it was present

    If <i>e</i> is not present, just returns <i>s</i>
*/
inline const std::string remove_from_end(const std::string& s, const std::string& e)
  { return ( ends_with(s, e) ? remove_from_end(s, e.length()) : s ); }

/*! \brief              Split a string into components
    \param  cs          original string
    \param  separator   separator string (typically a single character)
    \return             vector containing the separate components
*/
const std::vector<std::string> split_string(const std::string& cs, const std::string& separator);

/*! \brief              Split a string into components
    \param  cs          original string
    \param  separator   separator character
    \return             vector containing the separate components
*/
inline const std::vector<std::string> split_string(const std::string& cs, const char separator)
  { return split_string(cs, std::string(1, separator)); }

/*! \brief                  Split a string into equal-length records
    \param  cs              original string
    \param  record_length   length of each record
    \return                 vector containing the separate components

    Any non-full record at the end is silently discarded
*/
const std::vector<std::string> split_string(const std::string& cs, const unsigned int record_length);

/*! \brief                  Split a string into equal-length records
    \param  cs              original string
    \param  record_length   length of each record
    \return                 vector containing the separate components

    Any non-full record at the end is silently discarded
*/
inline const std::vector<std::string> split_string(const std::string& cs, const int record_length)
  { return split_string(cs, static_cast<unsigned int>(record_length)); }

#if 1
/*! \brief      Squash repeated occurrences of a character
    \param  cs  original string
    \param  c   character to squash
    \return     <i>cs</i>, but with all consecutive instances of <i>c</i> converted to a single instance
*/
const std::string squash(const std::string& cs, const char c = ' ');

/*! \brief      Squash repeated occurrences of a character
    \param  s   original string
    \param  c   character to squash
    \return     <i>s</i>, but with all consecutive instances of <i>c</i> converted to a single instance
*/
const std::string squash(std::string& s, const char c = ' ');
#endif

/*! \brief      Squash repeated occurrences of a character
    \param  s   original string
    \param  c   character to squash
    \return     <i>s</i>, but with all consecutive instances of <i>c</i> converted to a single instance
*/
//const std::string squash(const std::string&& s, const char c = ' ');

/*! \brief          Remove empty lines from a vector of lines
    \param  lines   the original vector of lines
    \return         <i>lines</i>, but with all empty lines removed

    If the line contains anything, even just whitespace, it is not removed
*/
const std::vector<std::string> remove_empty_lines(const std::vector<std::string>& lines);

/*! \brief              Split a string into lines
    \param  cs          original string
    \param  eol_marker  EOL marker
    \return             vector containing the separate lines
*/
inline const std::vector<std::string> to_lines(const std::string& cs, const std::string& eol_marker = EOL)
  { return split_string(cs, eol_marker); }

/*! \brief      Remove all instances of a specific leading character
    \param  cs  original string
    \param  c   leading character to remove (if present)
    \return     <i>cs</i> with any leading octets with the value <i>c</i> removed
*/
const std::string remove_leading(const std::string& cs, const char c);

/*! \brief      Remove leading spaces
    \param  cs  original string
    \return     <i>cs</i> with any leading spaces removed
*/
inline const std::string remove_leading_spaces(const std::string& cs)
  { return remove_leading(cs, ' '); }

/*! \brief      Remove all instances of a specific trailing character
    \param  cs  original string
    \param  c   trailing character to remove (if present)
    \return     <i>cs</i> with any trailing octets with the value <i>c</i> removed
*/
const std::string remove_trailing(const std::string& cs, const char c);

/*! \brief      Remove trailing spaces
    \param  cs  original string
    \return     <i>cs</i> with any trailing spaces removed
*/
inline const std::string remove_trailing_spaces(const std::string& cs)
  { return remove_trailing(cs, ' '); }

/*! \brief      Remove leading and trailing spaces
    \param  cs  original string
    \return     <i>cs</i> with any leading or trailing spaces removed
*/
inline const std::string remove_peripheral_spaces(const std::string& cs)
  { return remove_trailing_spaces(remove_leading_spaces(cs)); }

/*! \brief      Remove leading and trailing spaces
    \param  s   original string
    \return     <i>s</i> with any leading or trailing spaces removed
*/
inline const std::string remove_peripheral_spaces(std::string& s)
  { return remove_trailing_spaces(remove_leading_spaces(s)); }

/*! \brief      Remove leading and trailing spaces
    \param  t   container of strings
    \return     <i>t</i> with leading and trailing spaces removed from the individual elements
*/
template <typename T>
T remove_peripheral_spaces(T& t)
{ typename std::remove_const<T>::type rv;

  for_each(t.cbegin(), t.cend(), [&rv](const std::string& s) { rv.push_back(remove_peripheral_spaces(s)); } );

  return rv;
}

/*! \brief              Replace every instance of one string with another
    \param  s           string on which to operate
    \param  old_str     string to be replaced
    \param  new_str     replacement string
    \return             <i>s</i>, with every instance of <i>old_str</i> replaced by <i>new_str</i>
*/
template <typename T>
T replace(T& t, const std::string& s1, const std::string& s2)
{ typename std::remove_const<T>::type rv;

  for_each(t.cbegin(), t.cend(), [=, &rv](const std::string& s) { rv.push_back(replace(s, s1, s2)); } );

  return rv;
}

/*! \brief      Squash repeated occurrences of a character for each string in a container
    \param  t   original container of strings
    \param  c   character to squash
    \return     <i>t</i>, but with all consecutive instances of <i>c</i> converted to a single instance
*/
#if 1
template <typename T>
T squash(T& t, const char c = ' ')
{ //typename std::remove_const<std::forward<T>>::type rv;
  typename std::remove_const<T>::type rv;

  for_each(t.cbegin(), t.cend(), [=, &rv](const std::string& s) { rv.push_back(squash(s, c)); } );

  return rv;
}
#endif

//const std::string squash(const std::string& cs, const char c = ' ');
//const std::string squash(std::string& cs, const char c = ' ');

/*! \brief      Remove peripheral instances of a specific character
    \param  cs  original string
    \param  c   character to remove
    \return     <i>cs</i> with any leading or trailing instances of <i>c</i> removed
*/
inline const std::string remove_peripheral_character(const std::string& cs, const char c)
  { return remove_trailing(remove_leading(cs, c), c); }

/*! \brief                  Remove all instances of a particular char from a string
    \param  cs              original string
    \param  char_to_remove  character to be removed from <i>cs</i>
    \return                 <i>cs</i> with all instances of <i>char_to_remove</i> removed
*/
const std::string remove_char(const std::string& cs, const char char_to_remove);

/*! \brief                  Remove all instances of a particular char from a string
    \param  s               original string
    \param  char_to_remove  character to be removed from <i>cs</i>
    \return                 <i>cs</i> with all instances of <i>char_to_remove</i> removed
*/
inline const std::string remove_char(std::string& s, const char char_to_remove)
  { return remove_char(static_cast<const std::string>(s), char_to_remove); }

/*! \brief                  Remove all instances of a particular char from a container of strings
    \param  t               original container of strings
    \param  char_to_remove  character remove
    \return                 <i>t</i> with all instances of <i>char_to_remove</i> removed
*/
template <typename T>
T remove_char(T& t, const char char_to_remove)
{ typename std::remove_const<T>::type rv;

  for_each(t.cbegin(), t.cend(), [=, &rv](const std::string& cs) { rv.push_back(remove_char(cs, char_to_remove)); } );

  return rv;
}

/*! \brief                      Remove all instances of particular chars from a string
    \param  s                   original string
    \param  chars_to_remove     the characters to be removed from <i>s</i>
    \return                     <i>s</i> with all instances of the characters in <i>chars_to_remove</i> removed
*/
const std::string remove_chars(const std::string& s, const std::string& chars_to_remove);

/*! \brief                  Remove all instances of a particular char from all delimited substrings
    \param  cs              original string
    \param  char_to_remove  character to be removed from delimited substrings in <i>cs</i>
    \param  delim_1         opening delimiter
    \param  delim_2         closing delimiter
    \return                 <i>cs</i> with all instances of <i>char_to_remove</i> removed from inside substrings delimited by <i>delim_1</i> and <i>delim_2</i>
*/
const std::string remove_char_from_delimited_substrings(const std::string& cs, const char char_to_remove, const char delim_1, const char delim_2);

/*! \brief              Obtain a delimited substring
    \param  cs          original string
    \param  delim_1     opening delimiter
    \param  delim_2     closing delimiter
    \return             substring between <i>delim_1</i> and <i>delim_2</i>
  
    Returns the empty string if the delimiters do not exist, or if
    <i>delim_2</i> does not appear after <i>delim_1</i>. Returns only the
    first delimited substring if more than one exists.
*/
const std::string delimited_substring(const std::string& cs, const char delim_1, const char delim_2);

/*! \brief              Obtain all occurrences of a delimited substring
    \param  cs          original string
    \param  delim_1     opening delimiter
    \param  delim_2     closing delimiter
    \return             all substrings between <i>delim_1</i> and <i>delim_2</i>
*/
const std::vector<std::string> delimited_substrings(const std::string& cs, const char delim_1, const char delim_2);

/*! \brief          Join the elements of a string vector, using a provided separator
    \param  vec     vector of strings
    \param  sep     separator inserted between the elements of <i>vec</i>
    \return         all the elements of <i>vec</i>, concatenated, but with <i>sep</i> inserted between elements
*/
const std::string join(const std::vector<std::string>& vec, const std::string& sep /* = " " */);

/*! \brief          Join the elements of a string deque, using a provided separator
    \param  deq     deque of strings
    \param  sep     separator inserted between the elements of <i>vec</i>
    \return         all the elements of <i>vec</i>, concatenated, but with <i>sep</i> inserted between elements
*/
const std::string join(const std::deque<std::string>& deq, const std::string& sep /* = " " */);

/*! \brief          Join the elements of an array, using a provided separator
    \param  vt      array of elements to be joined
    \param  sep     separator inserted between the elements of <i>vec</i>
    \return         all the elements of <i>vt</i>, concatenated, but with <i>sep</i> inserted between elements
*/
template<typename T, size_t N>
const std::string join(const std::array<T, N>& vt, const std::string& sep)
{ std::string rv;

  if (vt.empty())
    return rv;

  for (unsigned int n = 0; n < vt.size() - 1; ++n)
  { const std::string s { to_string(vt[n]) };

    rv += (s + sep);
  }

  const std::string last_str { to_string(vt[vt.size() - 1]) };

  rv += last_str;

  return rv;
}

/*! \brief      Get the last character in a string
    \param  cs  source string
    \return     last character in <i>cs</i>

    Throws exception if <i>cs</i> is empty
*/
const char last_char(const std::string& cs);

/*! \brief      Get the penultimate character in a string
    \param  cs  source string
    \return     penultimate character in <i>cs</i>

    Throws exception if <i>cs</i> is empty or contains only one character
*/
const char penultimate_char(const std::string& cs);

/*! \brief      Get the antepenultimate character in a string
    \param  cs  source string
    \return     antepenultimate character in <i>cs</i>

    Throws exception if <i>cs</i> contains fewer than two characters
*/
const char antepenultimate_char(const std::string& cs);

/*! \brief          Get the terminating part of a string
    \param  cs      original string
    \param  n       number of characters to return
    \return         the last <i>n</i> characters of <i>cs</i>
*/
inline const std::string last(const std::string& cs, unsigned int n)
  { return (cs.length() < n ? cs : cs.substr(cs.length() - n)); }

/*! \brief              Get an environment variable
    \param  var_name    name of the environment variable
    \return             the contents of the environment variable <i>var_name</i>

    Returns the empty string if the variable <i>var_name</i> does not exist
*/
const std::string get_environment_variable(const std::string& var_name);

/*! \brief      Transform a string
    \param  cs  original string
    \param  pf  pointer to transformation function
    \return     <i>cs</i> with the transformation <i>*pf</i> applied
*/
const std::string transform_string(const std::string& cs, int(*pf)(int));

/*! \brief      Convert string to upper case
    \param  cs  original string
    \return     <i>cs</i> converted to upper case
*/
inline const std::string to_upper(const std::string& cs)
  { return transform_string(cs, std::toupper); }

/*! \brief      Convert string to lower case
    \param  cs  original string
    \return     <i>cs</i> converted to lower case
*/
inline const std::string to_lower(const std::string& cs)
  { return transform_string(cs, std::tolower); }

/*! \brief              Is a call a maritime mobile?
    \param  callsign    call to test
    \return             whether <i>callsign</i> appears to be a maritime mobile
*/
inline const bool is_maritime_mobile(const std::string& callsign)
  { return ( ends_with(to_upper(callsign), "/MM"s ) ); }

/*! \brief          Convert an integer to a character-separated string
    \param  n       number to convert
    \param  sep     string to act as the triplet separator
    \return         <i>n</i> with the separator <i>sep</i> separating each triplet

    Uses comma as separator if <i>sep</i> is empty.
*/
template <typename T>
const std::string separated_string(const T n, const std::string& sep = ","s)
{ const char separator { (sep.empty() ? ',' : sep[0]) };

  std::string tmp { to_string(n) };

  std::string rv;

  while (!tmp.empty())
  { for (unsigned int N = 0; N < 3 and !tmp.empty(); ++N)
    { rv = std::string(1, last_char(tmp)) + rv;
      tmp = tmp.substr(0, tmp.length() - 1);
    }

    if (!tmp.empty())
      rv = std::string(1, separator) + rv;
  }

  return rv;
}

/*! \brief      Convert an integer to a comma-separated string
    \param  n   number to convert
    \return     <i>n</i> with the separator <i>,</i> separating each triplet
*/
template <typename T>
inline const std::string comma_separated_string(const T n)
  { return separated_string(n); }

/*! \brief                  Get location of start of next word
    \param  str             string in which the next word is to be found
    \param  current_posn    position from which to search
    \return                 position of start of next word, beginning at position <i>current_posn</i> in <i>str</i>

    Returns <i>string::npos</i> if no word can be found
*/
const size_t next_word_posn(const std::string& str, const size_t current_posn);

/*! \brief      Get location of start all words
    \param  s   string to be analysed
    \return     positions of all the starts of words in <i>s</i>
*/
const std::vector<size_t> starts_of_words(const std::string& s);

/*! \brief          Get nth word in a string
    \param  s       string to be analysed
    \param  n       word number to be returned
    \param  wrt     value with respect to which <i>n</i> is counted
    \return         the <i>n</i>th word, counting with respect to <i>wrt</i>

    Returns <i>string::npos</i> if there is no <i>n</i>th word
*/
const std::string nth_word(const std::string& s, const unsigned int n, const unsigned int wrt = 0);

/*! \brief          Get the actual length, in bytes, of a UTF-8-encoded string
    \param  str     UTF-8 string to be analysed
    \return         number of bytes occupied by <i>str</i>

    See: https://stackoverflow.com/questions/4063146/getting-the-actual-length-of-a-utf-8-encoded-stdstring
    TODO: generalise using locales/facets, instead of assuming UTF-8
*/
const size_t n_chars(const std::string& str);

/*! \brief      Does a string contain a legal dotted-decimal IPv4 address
    \param  cs  original string
    \return     whether <i>cs</i> contains a legal dotted decimal IPv4 address
*/
const bool is_legal_ipv4_address(const std::string& cs);  

/*! \brief          Convert a four-byte value to a dotted decimal string
    \param  val     original value
    \return         dotted decimal string corresponding to <i>val</i>
*/
const std::string convert_to_dotted_decimal(const uint32_t val);

/*! \brief                  Is a string a legal value from a list?
    \param  value           target string
    \param  legal_values    all the legal values, separated by <i>separator</i>
    \param  separator       separator in the string <i>legal_values</i>
    \return                 whether <i>value</i> appears in <i>legal_values</i>
*/
const bool is_legal_value(const std::string& value, const std::string& legal_values, const std::string& separator);

/*! \brief          Is one call earlier than another, according to callsign sort order?
    \param  call1   first call
    \param  call2   second call
    \return         whether <i>call1</i> appears before <i>call2</i> in callsign sort order
*/
const bool compare_calls(const std::string& call1, const std::string& call2);

/*! \brief          Return a number with a particular number of decimal places
    \param  str     initial value
    \param  n       number of decimal places
    \return         <i>str</i> with <i>n</i> decimal places

    Assumes that <i>str</i> is a number
*/
const std::string decimal_places(const std::string& str, const int n);

/*! \brief          Return the longest line from a vector of lines
    \param  lines   the lines to search
    \return         the longest line in the vector <i>lines</i>
*/
const std::string longest_line(const std::vector<std::string>& lines);

/*! \brief          Deal with wprintw's idiotic insertion of newlines when reaching the right hand of a window
    \param  str     string to be reformatted
    \param  width   width of line in destination window
    \return         <i>str</i> reformatted for the window

    See http://stackoverflow.com/questions/7540029/wprintw-in-ncurses-when-writing-a-newline-terminated-line-of-exactly-the-same
*/
const std::string reformat_for_wprintw(const std::string& str, const int width);

/*! \brief          Deal with wprintw's idiotic insertion of newlines when reaching the right hand of a window
    \param  vecstr  vector of strings to be reformatted
    \param  width   width of line in destination window
    \return         <i>str</i> reformatted for the window

    See http://stackoverflow.com/questions/7540029/wprintw-in-ncurses-when-writing-a-newline-terminated-line-of-exactly-the-same
*/
const std::vector<std::string> reformat_for_wprintw(const std::vector<std::string>& vecstr, const int width);

/*! \brief      Remove all instances of a particular substring from a string
    \param  cs  original string
    \param  ss  substring to be removed
    \return     <i>cs</i>, with all instances of <i>ss</i> removed
*/
inline const std::string remove_substring(const std::string& cs, const std::string& ss)
  { return ( contains(cs, ss) ? replace(cs, ss, std::string()) : cs ); }

/*! \brief  Create a string of spaces
    \param  n   Length of string to be created
    \return String of <i>n</i> space characters
*/
inline const std::string space_string(const int n)
  { return (n > 0 ? create_string((char)32, n) : std::string()); }

/*! \brief              Get the base portion of a call
    \param  callsign    original callsign
    \return             the base portion of <i>callsign</i>

    For example, a call such as VP9/G4AMJ/P returns G4AMJ.
*/
const std::string base_call(const std::string& callsign);

/*! \brief              String to append to reach the next tab stop
    \param  str         source string
    \param  tab_size    number of spaces per tab stop
    \return             string to append to <i>str</i> to reach the next tab stop with spaces
*/
inline const std::string tab_spaces(const std::experimental::string_view str, const unsigned int tab_size = 8)
  { return ( space_string(tab_size - (str.length() % tab_size)) ); }

/*! \brief              Append spaces up to the next tab stop
    \param  str         source string
    \param  tab_size    number of spaces per tab stop
    \return             <i>str</i> padded out to the next tab stop with spaces
*/
inline const std::string append_tab_spaces(const std::experimental::string_view str, const unsigned int tab_size = 8)
  { return ( std::string(str) + tab_spaces(str, tab_size) ); }

/*! \brief              Add trailing slash to a directory name, if necessary
    \param  dirname     name of directory
    \return             <i>dirname</i>, but with a slash appended if it does not have one at the end
*/
inline const std::string dirname_with_slash(const std::string& dirname)
  { return ( dirname + ( (last_char(dirname) == '/') ? std::string() : "/"s ) ); }

// -------------------------------------- Errors  -----------------------------------

ERROR_CLASS(string_function_error);     ///< string_function error

#endif    // STRING_FUNCTIONS_H
