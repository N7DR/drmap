// $Id: macros.h 2 2019-10-03 18:02:04Z n7dr $

// Released under the GNU Public License, version 2
//   see: https://www.gnu.org/licenses/gpl-2.0.html

// Principal author: N7DR

// Copyright owners:
//    N7DR

#ifndef MACROS_H
#define MACROS_H

/*! \file   macros.h

    Macros and templates for drlog.
*/

#include <algorithm>
#include <chrono>
#include <experimental/functional>  // for not_fn
#include <future>
#include <iostream>
#include <map>
#include <set>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

/// Syntactic sugar for read/write access
#if (!defined(READ_AND_WRITE))

#define READ_AND_WRITE(y)                                       \
/*! Read access to _##y */                                      \
  inline const decltype(_##y)& y(void) const { return _##y; }   \
/*! Write access to _##y */                                     \
  inline void y(const decltype(_##y)& n) { _##y = n; }

#endif    // !READ_AND_WRITE

/// Syntactic sugar for read/write access with thread locking
#if (!defined(SAFE_READ_AND_WRITE))

#define SAFE_READ_AND_WRITE(y, z)                                           \
/*! Read access to _##y */                                                  \
  inline const decltype(_##y)& y(void) const { SAFELOCK(z); return _##y; }  \
/*! Write access to _##y */                                                 \
  inline void y(const decltype(_##y)& n) { SAFELOCK(z); _##y = n; }

#endif    // !SAFE_READ_AND_WRITE

/// Read and write with mutex that is part of the object
#if (!defined(SAFE_READ_AND_WRITE_WITH_INTERNAL_MUTEX))

#define SAFE_READ_AND_WRITE_WITH_INTERNAL_MUTEX(y, z)                                           \
/*! Read access to _##y */                                                  \
  inline const decltype(_##y)& y(void) { SAFELOCK(z); return _##y; }  \
/*! Write access to _##y */                                                 \
  inline void y(const decltype(_##y)& n) { SAFELOCK(z); _##y = n; }

#endif    // !SAFE_READ_AND_WRITE_WITH_INTERNAL_MUTEX

/// Syntactic sugar for read-only access
#if (!defined(READ))

#define READ(y)                                                 \
/*! Read-only access to _##y */                                 \
  inline const decltype(_##y)& y(void) const { return _##y; }

#endif    // !READ

/// Syntactic sugar for read-only access with thread locking
#if (!defined(SAFEREAD))

#define SAFEREAD(y, z)                                                      \
/*! Read-only access to _##y */                                             \
  inline const decltype(_##y)& y(void) const { SAFELOCK(z); return _##y; }

#endif    // !SAFEREAD

// alternative name for SAFEREAD
#if (!defined(SAFE_READ))

/// read with a mutex
#define SAFE_READ(y, z)                                                      \
/*! Read-only access to _##y */                                             \
  inline const decltype(_##y)& y(void) const { SAFELOCK(z); return _##y; }

#endif    // !SAFE_READ

/// read with mutex that is part of the object
#if (!defined(SAFEREAD_WITH_INTERNAL_MUTEX))

#define SAFEREAD_WITH_INTERNAL_MUTEX(y, z)                                                      \
/*! Read-only access to _##y */                                             \
  inline const decltype(_##y)& y(void) { SAFELOCK(z); return _##y; }

#endif    // !SAFEREAD_WITH_INTERNAL_MUTEX

// alternative name for SAFEREAD_WITH_INTERNAL_MUTEX
#if (!defined(SAFE_READ_WITH_INTERNAL_MUTEX))

/// read with an internal mutex
#define SAFE_READ_WITH_INTERNAL_MUTEX(y, z)                                                      \
/*! Read-only access to _##y */                                             \
  inline const decltype(_##y)& y(void) { SAFELOCK(z); return _##y; }

#endif    // !SAFE_READ_WITH_INTERNAL_MUTEX

/// Syntactic sugar for write access
#if (!defined(WRITE))

#define WRITE(y)                                       \
/*! Write access to _##y */                                     \
  inline void y(const decltype(_##y)& n) { _##y = n; }

#endif    // !READ_AND_WRITE

/// Error classes are all similar, so define a macro for them
#if (!defined(ERROR_CLASS))

#define ERROR_CLASS(z) \
\
class z : public x_error \
{ \
protected: \
\
public: \
\
/*! \brief      Construct from error code and reason \
    \param  n   error code \
    \param  s   reason \
*/ \
  inline z(const int n, const std::string& s = (std::string)"") : \
    x_error(n, s) \
  { } \
}

#endif      // !ERROR_CLASS

// classes for tuples... it seems like there should be a way to do this with TMP,
// but the level-breaking caused by the need to control textual names seems to make
// this impossible without resorting to #defines. So since I don't immediately see
// a way to avoid #defines completely while keeping the usual access syntax
// (i.e., obj.name()), we might as well use a sledgehammer and do everything with #defines.

// we could access with something like obj.at<name>, but that would mean a different access
// style for this kind of object as compared to ordinary classes using the READ and READ_AND_WRITE
// macros

/// tuple class (1) -- complete overkill
#define WRAPPER_1(nm, a0, a1)                          \
                                                       \
class nm : public std::tuple < a0 >                    \
{                                                      \
protected:                                             \
                                                       \
public:                                                \
                                                       \
  nm( a0 X )                                           \
    { std::get<0>(*this) = X;                          \
    }                                                  \
                                                       \
  inline const a0 a1(void) const                       \
    { return std::get<0>(*this); }                     \
                                                       \
  inline void a1(const a0 & var)                       \
    { std::get<0>(*this) = var; }                      \
}

/// tuple class (2)
#define WRAPPER_2(nm, a0, a1, b0, b1)                  \
                                                       \
class nm : public std::tuple < a0, b0 >                \
{                                                      \
protected:                                             \
                                                       \
public:                                                \
                                                       \
  nm( a0 X, b0 Y)                                      \
    { std::get<0>(*this) = X;                          \
      std::get<1>(*this) = Y;                          \
    }                                                  \
                                                       \
  inline const a0 a1(void) const                       \
    { return std::get<0>(*this); }                     \
                                                       \
  inline void a1(const a0 & var)                       \
    { std::get<0>(*this) = var; }                      \
                                                       \
  inline const b0 b1(void) const                       \
    { return std::get<1>(*this); }                     \
                                                       \
  inline void b1(const b0 & var)                       \
    { std::get<1>(*this) = var; }                      \
}

/// tuple class (2)
#define WRAPPER_2_NC(nm, a0, a1, b0, b1)               \
                                                       \
class nm : public std::tuple < a0, b0 >                \
{                                                      \
protected:                                             \
                                                       \
public:                                                \
                                                       \
  nm( a0 X, b0 Y)                                      \
    { std::get<0>(*this) = X;                          \
      std::get<1>(*this) = Y;                          \
    }                                                  \
                                                       \
  inline a0 a1(void) const                             \
    { return std::get<0>(*this); }                     \
                                                       \
  inline void a1(a0 var)                               \
    { std::get<0>(*this) = var; }                      \
                                                       \
  inline b0 b1(void) const                             \
    { return std::get<1>(*this); }                     \
                                                       \
  inline void b1(b0 var)                               \
    { std::get<1>(*this) = var; }                      \
}

/// tuple class (3)
#define WRAPPER_3(nm, a0, a1, b0, b1, c0, c1)          \
                                                       \
class nm : public std::tuple < a0, b0, c0 >            \
{                                                      \
protected:                                             \
                                                       \
public:                                                \
                                                       \
  nm( a0 X, b0 Y, c0 Z)                                \
    { std::get<0>(*this) = X;                          \
      std::get<1>(*this) = Y;                          \
      std::get<2>(*this) = Z;                          \
    }                                                  \
                                                       \
  nm( void ) { }                                       \
                                                       \
  inline const a0 a1(void) const                       \
    { return std::get<0>(*this); }                     \
                                                       \
  inline void a1(a0 var)                               \
    { std::get<0>(*this) = var; }                      \
                                                       \
  inline const b0 b1(void) const                       \
    { return std::get<1>(*this); }                     \
                                                       \
  inline void b1(b0 var)                               \
    { std::get<1>(*this) = var; }                      \
                                                       \
  inline const c0 c1(void) const                       \
    { return std::get<2>(*this); }                     \
                                                       \
  inline void c1(c0 var)                               \
    { std::get<2>(*this) = var; }                      \
};                                                     \
                                                       \
inline std::ostream& operator<<(std::ostream& ost, const nm& type)  \
{ ost << #nm << ": " << std::endl                                   \
      << #a1 << ": " << type.a1() << std::endl                      \
      << #b1 << ": " << type.b1() << std::endl                      \
      << #c1 << ": " << type.c1();                                  \
                                                                    \
  return ost;                                                       \
}

/// tuple class (3)
#define WRAPPER_3_NC(nm, a0, a1, b0, b1, c0, c1)        \
                                                        \
class nm : public std::tuple < a0, b0, c0 >             \
{                                                       \
protected:                                              \
                                                        \
public:                                                 \
                                                        \
  nm( a0 X, b0 Y, c0 Z)                                 \
    { std::get<0>(*this) = X;                           \
      std::get<1>(*this) = Y;                           \
      std::get<2>(*this) = Z;                           \
    }                                                   \
                                                        \
  inline a0 a1(void) const                              \
    { return std::get<0>(*this); }                      \
                                                        \
  inline void a1(a0 var)                                \
    { std::get<0>(*this) = var; }                       \
                                                        \
  inline b0 b1(void) const                              \
    { return std::get<1>(*this); }                      \
                                                        \
  inline void b1(b0 var)                                \
    { std::get<1>(*this) = var; }                       \
                                                        \
  inline c0 c1(void) const                              \
    { return std::get<2>(*this); }                      \
                                                        \
  inline void c1(c0 var)                                \
    { std::get<2>(*this) = var; }                       \
}

/// tuple class (3)
#define WRAPPER_3_SERIALIZE(nm, a0, a1, b0, b1, c0, c1)                     \
                                                                            \
class nm : public std::tuple < a0, b0, c0 >                                 \
{                                                                           \
protected:                                                                  \
                                                                            \
public:                                                                     \
                                                                            \
  nm( a0 X, b0 Y, c0 Z)                                                     \
    { std::get<0>(*this) = X;                                               \
      std::get<1>(*this) = Y;                                               \
      std::get<2>(*this) = Z;                                               \
    }                                                                       \
                                                                            \
  nm( void ) { }                                                            \
                                                                            \
  inline const a0 a1(void) const                                            \
    { return std::get<0>(*this); }                                          \
                                                                            \
  inline void a1(a0 var)                                                    \
    { std::get<0>(*this) = var; }                                           \
                                                                            \
  inline const b0 b1(void) const                                            \
    { return std::get<1>(*this); }                                          \
                                                                            \
  inline void b1(b0 var)                                                    \
    { std::get<1>(*this) = var; }                                           \
                                                                            \
  inline const c0 c1(void) const                                            \
    { return std::get<2>(*this); }                                          \
                                                                            \
  inline void c1(c0 var)                                                    \
    { std::get<2>(*this) = var; }                                           \
                                                                            \
    template<typename Archive>                                              \
  void serialize(Archive& ar, const unsigned version)                       \
    { ar & std::get<0>(*this)                                               \
         & std::get<1>(*this)                                               \
         & std::get<2>(*this);                                              \
    }                                                                       \
}

/// tuple class (4)
#define WRAPPER_4_NC(nm, a0, a1, b0, b1, c0, c1, d0, d1)                    \
                                                                            \
class nm : public std::tuple < a0, b0, c0, d0 >                             \
{                                                                           \
protected:                                                                  \
                                                                            \
public:                                                                     \
                                                                            \
  nm( a0 X, b0 Y, c0 Z, d0 A)                                               \
    { std::get<0>(*this) = X;                                               \
      std::get<1>(*this) = Y;                                               \
      std::get<2>(*this) = Z;                                               \
      std::get<3>(*this) = A;                                               \
}                                                                           \
                                                                            \
  inline a0 a1(void) const                                                  \
    { return std::get<0>(*this); }                                          \
                                                                            \
  inline void a1(a0 var)                                                    \
    { std::get<0>(*this) = var; }                                           \
                                                                            \
  inline b0 b1(void) const                                                  \
    { return std::get<1>(*this); }                                          \
                                                                            \
  inline void b1(b0 var)                                                    \
    { std::get<1>(*this) = var; }                                           \
                                                                            \
  inline c0 c1(void) const                                                  \
    { return std::get<2>(*this); }                                          \
                                                                            \
  inline void c1(c0 var)                                                    \
    { std::get<2>(*this) = var; }                                           \
                                                                            \
  inline d0 d1(void) const                                                  \
    { return std::get<3>(*this); }                                          \
                                                                            \
  inline void d1(d0 var)                                                    \
    { std::get<3>(*this) = var; }                                           \
}

/// tuple class (4)
#define WRAPPER_4_SERIALIZE(nm, a0, a1, b0, b1, c0, c1, d0, d1)             \
                                                                            \
class nm : public std::tuple < a0, b0, c0, d0  >                            \
{                                                                           \
protected:                                                                  \
                                                                            \
public:                                                                     \
                                                                            \
  nm( a0 X, b0 Y, c0 Z, d0 A)                                               \
    { std::get<0>(*this) = X;                                               \
      std::get<1>(*this) = Y;                                               \
      std::get<2>(*this) = Z;                                               \
      std::get<3>(*this) = A;                                               \
    }                                                                       \
                                                                            \
  nm( void ) { }                                                            \
                                                                            \
  inline const a0 a1(void) const                                            \
    { return std::get<0>(*this); }                                          \
                                                                            \
  inline void a1(a0 var)                                                    \
    { std::get<0>(*this) = var; }                                           \
                                                                            \
  inline const b0 b1(void) const                                            \
    { return std::get<1>(*this); }                                          \
                                                                            \
  inline void b1(b0 var)                                                    \
    { std::get<1>(*this) = var; }                                           \
                                                                            \
  inline const c0 c1(void) const                                            \
    { return std::get<2>(*this); }                                          \
                                                                            \
  inline void c1(c0 var)                                                    \
    { std::get<2>(*this) = var; }                                           \
                                                                            \
  inline const d0 d1(void) const                                            \
    { return std::get<3>(*this); }                                          \
                                                                            \
  inline void d1(d0 var)                                                    \
    { std::get<3>(*this) = var; }                                           \
                                                                            \
template<typename Archive>                                                  \
  void serialize(Archive& ar, const unsigned version)                       \
    { ar & std::get<0>(*this)                                               \
         & std::get<1>(*this)                                               \
         & std::get<2>(*this)                                               \
         & std::get<3>(*this);                                              \
    }                                                                       \
};                                                                          \
                                                                            \
inline std::ostream& operator<<(std::ostream& ost, const nm& type)          \
{ ost << #nm << ": " << std::endl                                           \
      << #a1 << ": " << type.a1() << std::endl                              \
      << #b1 << ": " << type.b1() << std::endl                              \
      << #c1 << ": " << type.c1() << std::endl                              \
      << #d1 << ": " << type.d1();                                          \
                                                                            \
  return ost;                                                               \
}

/// tuple class (5)
#define WRAPPER_5_NC(nm, a0, a1, b0, b1, c0, c1, d0, d1, e0, e1)            \
                                                                            \
class nm : public std::tuple < a0, b0, c0, d0, e0 >                         \
{                                                                           \
protected:                                                                  \
                                                                            \
public:                                                                     \
                                                                            \
  nm( a0 X, b0 Y, c0 Z, d0 A, e0 B)                                         \
    { std::get<0>(*this) = X;                                               \
      std::get<1>(*this) = Y;                                               \
      std::get<2>(*this) = Z;                                               \
      std::get<3>(*this) = A;                                               \
      std::get<4>(*this) = B;                                               \
    }                                                                       \
                                                                            \
  nm(void) { }                                                              \
                                                                            \
  inline a0 a1(void) const                                                  \
    { return std::get<0>(*this); }                                          \
                                                                            \
  inline void a1(a0 var)                                                    \
    { std::get<0>(*this) = var; }                                           \
                                                                            \
  inline b0 b1(void) const                                                  \
    { return std::get<1>(*this); }                                          \
                                                                            \
  inline void b1(b0 var)                                                    \
    { std::get<1>(*this) = var; }                                           \
                                                                            \
  inline c0 c1(void) const                                                  \
    { return std::get<2>(*this); }                                          \
                                                                            \
  inline void c1(c0 var)                                                    \
    { std::get<2>(*this) = var; }                                           \
                                                                            \
  inline d0 d1(void) const                                                  \
    { return std::get<3>(*this); }                                          \
                                                                            \
  inline void d1(d0 var)                                                    \
    { std::get<3>(*this) = var; }                                           \
                                                                            \
  inline e0 e1(void) const                                                  \
    { return std::get<4>(*this); }                                          \
                                                                            \
  inline void e1(e0 var)                                                    \
    { std::get<4>(*this) = var; }                                           \
}

/// tuple class (6)
#define WRAPPER_6_NC(nm, a0, a1, b0, b1, c0, c1, d0, d1, e0, e1, f0, f1)    \
                                                                            \
class nm : public std::tuple < a0, b0, c0, d0, e0, f0 >                     \
{                                                                           \
protected:                                                                  \
                                                                            \
public:                                                                     \
                                                                            \
  nm( a0 X, b0 Y, c0 Z, d0 A, e0 B, f0 C)                                   \
    { std::get<0>(*this) = X;                                               \
      std::get<1>(*this) = Y;                                               \
      std::get<2>(*this) = Z;                                               \
      std::get<3>(*this) = A;                                               \
      std::get<4>(*this) = B;                                               \
      std::get<5>(*this) = C;                                               \
}                                                                           \
                                                                            \
  inline a0 a1(void) const                                                  \
    { return std::get<0>(*this); }                                          \
                                                                            \
  inline void a1(a0 var)                                                    \
    { std::get<0>(*this) = var; }                                           \
                                                                            \
  inline b0 b1(void) const                                                  \
    { return std::get<1>(*this); }                                          \
                                                                            \
  inline void b1(b0 var)                                                    \
    { std::get<1>(*this) = var; }                                           \
                                                                            \
  inline c0 c1(void) const                                                  \
    { return std::get<2>(*this); }                                          \
                                                                            \
  inline void c1(c0 var)                                                    \
    { std::get<2>(*this) = var; }                                           \
                                                                            \
  inline d0 d1(void) const                                                  \
    { return std::get<3>(*this); }                                          \
                                                                            \
  inline void d1(d0 var)                                                    \
    { std::get<3>(*this) = var; }                                           \
                                                                            \
  inline e0 e1(void) const                                                  \
    { return std::get<4>(*this); }                                          \
                                                                            \
  inline void e1(e0 var)                                                    \
    { std::get<4>(*this) = var; }                                           \
                                                                            \
  inline f0 f1(void) const                                                  \
    { return std::get<5>(*this); }                                          \
                                                                            \
  inline void f1(f0 var)                                                    \
    { std::get<5>(*this) = var; }                                           \
}

/// tuple class (7)
#define WRAPPER_7_NC(nm, a0, a1, b0, b1, c0, c1, d0, d1, e0, e1, f0, f1, g0, g1)    \
                                                                                    \
class nm : public std::tuple < a0, b0, c0, d0, e0, f0, g0 >                         \
{                                                                                   \
protected:                                                                          \
                                                                                    \
public:                                                                             \
                                                                                    \
  nm( a0 X, b0 Y, c0 Z, d0 A, e0 B, f0 C, g0 D)                                     \
    { std::get<0>(*this) = X;                                                       \
      std::get<1>(*this) = Y;                                                       \
      std::get<2>(*this) = Z;                                                       \
      std::get<3>(*this) = A;                                                       \
      std::get<4>(*this) = B;                                                       \
      std::get<5>(*this) = C;                                                       \
      std::get<6>(*this) = D;                                                       \
}                                                                                   \
                                                                                    \
  inline a0 a1(void) const                                                          \
    { return std::get<0>(*this); }                                                  \
                                                                                    \
  inline void a1(a0 var)                                                            \
    { std::get<0>(*this) = var; }                                                   \
                                                                                    \
  inline b0 b1(void) const                                                          \
    { return std::get<1>(*this); }                                                  \
                                                                                    \
  inline void b1(b0 var)                                                            \
    { std::get<1>(*this) = var; }                                                   \
                                                                                    \
  inline c0 c1(void) const                                                          \
    { return std::get<2>(*this); }                                                  \
                                                                                    \
  inline void c1(c0 var)                                                            \
    { std::get<2>(*this) = var; }                                                   \
                                                                                    \
  inline d0 d1(void) const                                                          \
    { return std::get<3>(*this); }                                                  \
                                                                                    \
  inline void d1(d0 var)                                                            \
    { std::get<3>(*this) = var; }                                                   \
                                                                                    \
  inline e0 e1(void) const                                                          \
    { return std::get<4>(*this); }                                                  \
                                                                                    \
  inline void e1(e0 var)                                                            \
    { std::get<4>(*this) = var; }                                                   \
                                                                                    \
  inline f0 f1(void) const                                                          \
    { return std::get<5>(*this); }                                                  \
                                                                                    \
  inline void f1(f0 var)                                                            \
    { std::get<5>(*this) = var; }                                                   \
                                                                                    \
  inline g0 g1(void) const                                                          \
    { return std::get<6>(*this); }                                                  \
                                                                                    \
  inline void g1(g0 var)                                                            \
    { std::get<6>(*this) = var; }                                                   \
}

/// tuple class (8)
#define WRAPPER_8_NC(nm, a0, a1, b0, b1, c0, c1, d0, d1, e0, e1, f0, f1, g0, g1, h0, h1)    \
                                                                                            \
class nm : public std::tuple < a0, b0, c0, d0, e0, f0, g0, h0 >                             \
{                                                                                           \
protected:                                                                                  \
                                                                                            \
public:                                                                                     \
                                                                                            \
  nm( a0 X, b0 Y, c0 Z, d0 A, e0 B, f0 C, g0 D, h0 E)                                       \
    { std::get<0>(*this) = X;                                                               \
      std::get<1>(*this) = Y;                                                               \
      std::get<2>(*this) = Z;                                                               \
      std::get<3>(*this) = A;                                                               \
      std::get<4>(*this) = B;                                                               \
      std::get<5>(*this) = C;                                                               \
      std::get<6>(*this) = D;                                                               \
      std::get<7>(*this) = E;                                                               \
}                                                                                           \
                                                                                            \
  inline a0 a1(void) const                                                                  \
    { return std::get<0>(*this); }                                                          \
                                                                                            \
  inline void a1(a0 var)                                                                    \
    { std::get<0>(*this) = var; }                                                           \
                                                                                            \
  inline b0 b1(void) const                                                                  \
    { return std::get<1>(*this); }                                                          \
                                                                                            \
  inline void b1(b0 var)                                                                    \
    { std::get<1>(*this) = var; }                                                           \
                                                                                            \
  inline c0 c1(void) const                                                                  \
    { return std::get<2>(*this); }                                                          \
                                                                                            \
  inline void c1(c0 var)                                                                    \
    { std::get<2>(*this) = var; }                                                           \
                                                                                            \
  inline d0 d1(void) const                                                                  \
    { return std::get<3>(*this); }                                                          \
                                                                                            \
  inline void d1(d0 var)                                                                    \
    { std::get<3>(*this) = var; }                                                           \
                                                                                            \
  inline e0 e1(void) const                                                                  \
    { return std::get<4>(*this); }                                                          \
                                                                                            \
  inline void e1(e0 var)                                                                    \
    { std::get<4>(*this) = var; }                                                           \
                                                                                            \
  inline f0 f1(void) const                                                                  \
    { return std::get<5>(*this); }                                                          \
                                                                                            \
  inline void f1(f0 var)                                                                    \
    { std::get<5>(*this) = var; }                                                           \
                                                                                            \
  inline g0 g1(void) const                                                                  \
    { return std::get<6>(*this); }                                                          \
                                                                                            \
  inline void g1(g0 var)                                                                    \
    { std::get<6>(*this) = var; }                                                           \
                                                                                            \
  inline h0 h1(void) const                                                                  \
    { return std::get<7>(*this); }                                                          \
                                                                                            \
  inline void h1(h0 var)                                                                    \
    { std::get<7>(*this) = var; }                                                           \
}

/*! \brief      Is an object a member of a set?
    \param  s   set to be tested
    \param  v   object to be tested for membership
    \return     Whether <i>t</i> is a member of <i>s</i>
*/
template <class T>
const bool operator<(const std::set<T>& s, const T& v)
  { return s.find(v) != s.cend(); }

/*! \brief      Is an object a member of an unordered set?
    \param  s   unordered set to be tested
    \param  v   object to be tested for membership
    \return     Whether <i>t</i> is a member of <i>s</i>
*/
template <class T>
const bool operator<(const std::unordered_set<T>& s, const T& v)
  { return s.find(v) != s.cend(); }

/*! \brief                      Invert a mapping from map<T, set<T> > to map<T, set<T> >, where final keys are the elements of the original set
    \param  original_mapping    original mapping
    \return                     inverted mapping
*/
template <typename M>  // M = map<T, set<T> >
auto INVERT_MAPPING(const M& original_mapping) -> std::map<typename M::key_type, typename M::key_type>
{ std::map<typename M::key_type, typename M::key_type> rv;

  for (auto cit = original_mapping.cbegin(); cit != original_mapping.cend(); ++cit)
  { for (const auto& p : cit->second)
      rv.insert( { p, cit->first } );
  }

  return rv;
}

// syntactic suger for time-related use
//typedef std::chrono::duration<long, std::centi> centiseconds;           ///< hundredths of a second
//typedef std::chrono::duration<long, std::deci>  deciseconds;            ///< tenths of a second
using centiseconds =  std::chrono::duration<long, std::centi>;           ///< hundredths of a second
using deciseconds  =  std::chrono::duration<long, std::deci>;            ///< tenths of a second

#if 0
template <class D, class P> // P = parameter; D = data in the database
class cached_data
{
protected:

  std::map<P, D> _db;

  const D (*_lookup_fn)(const P&);

public:
  explicit cached_data(const D (*fn)(const P&)) :
    _lookup_fn(fn)
  { }

  D data(P& param)
    { auto it = _db.find(param);

      if (it != _db.end())
        return it->second;

      D value = _lookup_fn(param);

      _db.insert( { param, value } );
      return value;
    }

  D overwrite_data(P& param)
    { auto it = _db.find(param);

      if (it != _db.end())
        return it->second;

      D value = _lookup_fn(param);

      _db[param] = value;
      return value;
    }

  void erase(P& param)
  { auto it = _db.find(param);

    if (it == _db.end())
      return;

    _db.erase(param);
  }
};
#endif

/*! \class  RANGE
    \brief  allow easy execution of a loop a predetermined number of times

    This does throw a warning, but I can't think of a better way to
    execute a loop a predetermined number of times. C++14 might be going
    to provide a better mechanism.

    See also the UNUSED template below
*/
template <typename T>
class RANGE : public std::vector<T>
{
public:

/*! \brief      Generate a range
    \param  v1  lowest value
    \param  v2  highest value
*/
  RANGE(const T& v1, const T& v2)
  { if (v1 > v2)
    { T value = v1;

      while (value != v2)
      { this->push_back(value--);
      }

      this->push_back(v2);
    }
    else
    { for (T value = v1; value <= v2; ++value)
        this->push_back(value);
    }
  }
};

/// Syntactic sugar to avoid the "unused variable" warning when using the RANGE template, until C++ provides a proper way to have unused range-based loop variables
template<typename Unused>
inline void UNUSED( Unused&& )
{ }

/*! \class  accumulator
    \brief  accumulate values, and inform when a threshold is reached
*/
template <typename T>
class accumulator
{
protected:

  std::map<T, unsigned int> _values;                ///< all the known values, with the number of times it's been added
  unsigned int              _threshold;             ///< threshold value

public:

/// default constructor
  accumulator(const unsigned int thold = 1) :
    _threshold(thold)
  { }

  READ_AND_WRITE(threshold);                        ///< threshold value

/*! \brief          Add a value or increment it a known number of times
    \param  val     value to add or increment
    \param  n       number of times to add it
    \return         whether final number of times <i>value</i> has been added is at or greater than the threshold
*/
  const bool add(const T& val, const int n = 1)
  { if (_values.find(val) == _values.end())
      _values.insert( { val, n } );
    else
      _values[val] += n;

    return (_values[val] >= _threshold);
  }

/*! \brief          Number of times a value has been added
    \param  val     target value
    \return         total number of times <i>value</i> has been added
*/
  const unsigned int value(const T& val) const
  { if (_values.find(val) == _values.cend())
      return 0;

    return _values.at(val);
  }
};

// convenient syntactic sugar for some STL functions

/*! \brief          Write a <i>map<key, value></i> object to an output stream
    \param  ost     output stream
    \param  mp      object to write
    \return         the output stream
*/
template <class T1, class T2>
std::ostream& operator<<(std::ostream& ost, const std::map<T1, T2>& mp)
{ for (typename std::map<T1, T2>::const_iterator cit = mp.begin(); cit != mp.end(); ++cit)
    ost << "map[" << cit->first << "]: " << cit->second << std::endl;

  return ost;
}

/*! \brief          Apply a function to all in a container
    \param  first   container
    \param  fn      function
    \return         <i>fn</i>
*/
template<class Input, class Function>
inline Function FOR_ALL(Input& first, Function fn)
  { return (std::for_each(first.begin(), first.end(), fn)); }

/*! \brief          Copy all in a container to another container
    \param  first   initial container
    \param  oi      iterator on final container
    \return         <i>oi</i>
*/
template<class Input, class OutputIterator>
inline OutputIterator COPY_ALL(Input& first, OutputIterator oi)
  { return (std::copy(first.begin(), first.end(), oi)); }

/*! \brief          Remove values in a container that match a predicate, and resize the container
    \param  first   container
    \param  pred    predicate to apply

    Does not work for maps
*/
template <class Input, class UnaryPredicate>
inline void REMOVE_IF_AND_RESIZE(Input& first, UnaryPredicate pred)
  { first.erase(std::remove_if(first.begin(), first.end(), pred), first.end()); }

/*! \brief          Keep only values in a container that match a predicate, and resize the container
    \param  first   container
    \param  pred    predicate to apply

    Does not work for maps
*/
template <class Input, class UnaryPredicate>
inline void KEEP_IF_AND_RESIZE(Input& first, UnaryPredicate pred)
  { first.erase(std::remove_if(first.begin(), first.end(), std::experimental::not_fn(pred)), first.end()); }

// https://stackoverflow.com/questions/800955/remove-if-equivalent-for-stdmap
// there should be some way to choose this function instead of the prior one, based on
// traits, but I can't figure out a way to tell whether T is a map
//template< typename ContainerT, typename PredicateT >
//void MAP_REMOVE_IF( ContainerT& items, const PredicateT& predicate ) {
//  for( auto it = items.begin(); it != items.end(); ) {
//    if( predicate(*it) ) it = items.erase(it);
//    else ++it;
//  }
//};

/*! \brief          Remove map values that match a predicate, and resize the map
    \param  items   map
    \param  pred    predicate to apply

    Does not work for maps
*/
template< typename K, typename V, typename PredicateT >
void REMOVE_IF_AND_RESIZE( std::map<K, V>& items, const PredicateT& pred )
{ for( auto it = items.begin(); it != items.end(); )
  { if( pred(*it) )
      it = items.erase(it);
    else
      ++it;
  }
};

/*! \brief      Reverse the contents of a container
    \param  v   container
*/
template <class Input>
inline void REVERSE(Input& v)
  { std::reverse(v.begin(), v.end()); }

/*! \brief          Append from one container to another if a predicate is met
    \param  s       source container
    \param  d       destination container
    \param  pred    predicate
*/
template <class Input, class Output, typename PredicateT>
inline void APPEND_IF(Input& s, Output& d, const PredicateT& pred)
  { std::copy_if(s.begin(), s.end(), std::back_inserter(d), pred); }

/*! \brief          Create a container and append from another if a predicate is met
    \param  s       source container
    \param  pred    predicate
    \return         The new container

    Called as, for example, CREATE_AND_FILL<vector<string>>(in_vec, [](const string& s) { return (s == "BURBLE"s); } );
*/
template <typename Output, typename Input, typename PredicateT>
auto CREATE_AND_FILL(Input& s, const PredicateT& pred) -> const Output
{ Output rv;

  std::copy_if(s.begin(), s.end(), std::back_inserter(rv), pred);

  return rv;
}

/*! \brief          Find first value in a container that matches a predicate
    \param  v       container
    \param  pred    (boolean) predicate to apply
    \return         first value in <i>v</i> for which <i>pred</i> is true
*/
template <typename Input, typename UnaryPredicate>
inline auto FIND_IF(Input& v, UnaryPredicate pred) -> typename Input::iterator
  { return std::find_if(v.begin(), v.end(), pred); }

/*! \brief          Find first value in a container that matches a predicate
    \param  v       container (const)
    \param  pred    (boolean) predicate to apply
    \return         first value in <i>v</i> for which <i>pred</i> is true
*/
template <typename Input, typename UnaryPredicate>
inline auto FIND_IF(const Input& v, UnaryPredicate pred) -> typename Input::const_iterator
  { return std::find_if(v.cbegin(), v.cend(), pred); }

/*! \brief              Bound a value within limits
    \param  val         value to bound
    \param  low_val     lower bound
    \param  high_val    upper bound
    \return             max(min(<i>val</i>, <i>max_val</i>), <i>min_val</i>)
*/
template <typename T>
inline const T LIMIT(const T val, const T low_val, const T high_val)
  { return (val < low_val ? low_val : (val > high_val ? high_val : val)); }

/*! \brief              Bound a value within limits
    \param  val         value to bound
    \param  low_val     lower bound
    \param  high_val    upper bound
    \return             max(min(<i>val</i>, <i>max_val</i>), <i>min_val</i>)
*/
template <typename T, typename U, typename V>
inline const T LIMIT(const T val, const U low_val, const V high_val)
  { return (val < static_cast<T>(low_val) ? static_cast<T>(low_val) : (val > static_cast<T>(high_val) ? static_cast<T>(high_val) : val)); }

/*! \brief              Return first true element in vector of pair<bool, value> elements
    \param  vec         the vector
    \param  def         value to return of no elements are true
    \return             either the first true element or, if none, <i>def</i>
*/
template <typename T>
const T SELECT_FIRST_TRUE(std::vector<std::pair<bool, T>>& vec, const T& def)
{ for (const auto& element : vec)
  { if (element.first)
      return (element.second);
  }

  return def;
}

/*! \brief              Execute the same (void) function on multiple threads
    \param  n_threads   the number of threads on which to run the function <i>fn</i>
    \param  fn          the function to execute
    \param  args        arguments to <i>fn</i>

    This can be useful, for example, when reading large files, each line of which
    needs non-negligible processing and the order of processing is unimportant
*/
template <typename Function, typename... Args>
void EXECUTE_FUNCTION_MT(const unsigned int n_threads, Function&& fn, Args&&... args)
{ std::vector<std::future<void>> vec_futures;           // place to store the (void) futures

  for (unsigned int n = 0; n < n_threads; ++n)
    vec_futures.emplace_back(async(std::launch::async, fn, args...));

  for (auto& this_future : vec_futures)
    this_future.get();                                  // .get() blocks until the future is available
}

/*! \brief              Obtain a copy of the element with the minimum value in a container
    \param  container   the container
    \return             copy of element of <i>container</i> with the minimum value
*/
template <typename C>
inline auto MIN_ELEMENT(const C& container)
  { return ( *(std::min_element(begin(container), end(container))) ); }

/*! \brief              Obtain a copy of the element with the maximum value in a container
    \param  container   the container
    \return             copy of element of <i>container</i> with the maximum value
*/
template <typename C>
inline auto MAX_ELEMENT(const C& container)
  { return ( *(std::max_element(begin(container), end(container))) ); }

#endif    // MACROS_H
