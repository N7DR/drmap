// $Id: command_line.h 2 2019-10-03 18:02:04Z n7dr $

// Source code copyright 2003, 2005 IPfonix, Inc.
// Unauthorized copying strictly prohibited

#ifndef COMMANDLINEH
#define COMMANDLINEH

#include <string>

/*! \file   command_line.h

    API for managing the command line
*/

/*! \class  command_line
    \brief  Class that implements management of the command line
*/

class command_line
{
protected:
  unsigned int _argc;       ///< Number of arguments

// these can't be const because of the = operator
  char** _argv;             ///< Pointers to arguments

  std::string* _arg;        ///< Pointers to arguments

/// internal initialisation function
  void _init(void);

public:

/*! \brief          Constructor
    \param  argc    number of arguments
    \param  argv    pointer to array of individual arguments
*/
inline command_line(int argc, char** argv) :
    _argc(argc),
    _argv((char**)argv)
  { _init(); }

/*!	\brief	        Copy constructor
	\param	obj 	object to be copied
*/
inline command_line(const command_line& cl) :
    _argc(cl._argc),
    _argv(cl._argv)
  { _init(); }

/*!	\brief	Destructor
*/
  inline virtual ~command_line(void)
    { delete [] _arg; }

/// command_line = command_line
  void operator=(const command_line&);

/*! \brief      Return parameter number (wrt 1)
    \param  n   parameter number
    \return     the value of the <i>n</i>th parameter

    If the value of <i>n</i> does not correspond to a parameter that was actually present, this functions throws an x_command_line_invalid_parameter()
*/
  const std::string parameter(const unsigned int n) const;

/*! \brief      Obtain the name of the program
    \return     the name of the program
*/
  inline const std::string program_name(void) const
    { return _arg[0]; }

/*! \brief      Obtain the base name of the program
    \return     the base name of the program (i.e., with no "/" characters)
*/
  const std::string base_program_name(void) const;

/*! \brief      Obtain the number of parameters passed to the program
    \return     the number of parameters
*/
  inline const int n_parameters(void) const
    { return (_argc - 1); }

/*! \brief  Convert the entire command line to lower case
*/
  void tolower(void);

/*! \brief  Convert the entire command line to lower case
*/
  inline void to_lower(void)
    { tolower(); }

/*! \brief  Convert the entire command so that the case matches exactly what was originally passed to the program
*/
  void tooriginal(void);

/*! \brief  Convert the entire command so that the case matches exactly what was originally passed to the program
*/
  inline void to_original(void)
    { tooriginal(); }

/*! \brief  Convert the entire command line to upper case
*/
  void toupper(void);

/*! \brief  Convert the entire command line to upper case
*/
  inline void to_upper(void)
    { toupper(); }

/*! \brief      Convert a particular parameter to lower case
    \param  n   parameter number to convert (wrt 0)
*/
  void tolower(const unsigned int n);

/*! \brief      Convert a particular parameter to lower case
    \param  n   parameter number to convert (wrt 0)
*/
  inline void to_lower(const unsigned int n)
    { tolower(n); }

/*! \brief      Convert a particular parameter to its original case
    \param  n   parameter number to convert (wrt 0)
*/
  void tooriginal(const unsigned int n);

/*! \brief      Convert a particular parameter to its original case
    \param  n   parameter number to convert (wrt 0)
*/
  inline void to_original(const unsigned int n)
    { tooriginal(); }

/*!     \brief  Convert a particular parameter to upper case
        \param  n  Parameter number to convert (wrt 0) 
*/
  void toupper(const unsigned int n);

/*!     \brief  Is a particular value present?
        \param  v       Value for which to look
        \return Whether the value corresponding to <i>v</i> is present
        
        A "value" is something like a parameter to a -xxx option. If, for example, value_present("-xxx") is TRUE, it means that -xxx is present, and a value follows it  
*/
  const bool value_present(const std::string& v) const;

/*!     \brief  Return a particular value
        \param  v       Value to return
        \return The value
        
        A "value" is something like a parameter to a -xxx option. If, for example, the command line contains "-xxx burble", then value("-xxx") will return "burble"  
*/
  const std::string value(const std::string& v) const;

/*!     \brief  Is a particular parameter present?
        \param  p       Parameter for which to look
        \return Whether the parameter <i>p</i> is present
        
        A "parameter" is an actual parameter that appears on the command line.
*/
  const bool parameter_present(const std::string& p) const;

/*! \brief          Return a particular value if it's present
    \param  v       Value to return
    \return         The value, if it's present; otherwise, string()

    A "value" is something like a parameter to a -xxx option. If, for example, the command line contains "-xxx burble", then value("-xxx") will return "burble"
*/
    inline const std::string value_if_present(const std::string& v) const
      { return (value_present(v) ? value(v) : std::string() ); }
};

// ---------------------------  exceptions  ----------------------

/*! \class  x_command_line_invalid_parameter
    \brief  Trivial class for exceptions in command line processing
*/

class x_command_line_invalid_parameter
{
};

#endif    // !COMMANDLINEH
