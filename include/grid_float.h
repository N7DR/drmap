// $Id: grid_float.h 15 2019-11-03 15:02:05Z n7dr $

// Released under the GNU Public License, version 2

// Principal author: N7DR

// Copyright owners:
//    N7DR

#ifndef GRID_FLOAT_H
#define GRID_FLOAT_H

#include "string_functions.h"

#include <cmath>
#include <fstream>
#include <iostream>     // for writing to cout, etc.
#include <string>

/*! \file   grid_float.h

    USGS NED GridFloat tiles
*/

// error numbers
constexpr int GRID_FLOAT_NODATA { -1 };

constexpr double RE   { 6371000.0 };                  // radius in m
constexpr double PI   { 3.14159265358979 };
constexpr double DTOR { PI / 180.0 };
constexpr double RTOD { 1.0 / DTOR };

/*! \brief                      Download a tile from the USGS if we don't already have it
    \param  llc                 the llcode [lat * 1000 + (+ve)long]
    \param  local_directory     the local directory containing USGS files
*/
void download_if_necessary(const int llc, const std::string& local_directory);

// https://www.loc.gov/preservation/digital/formats/fdd/fdd000422.shtml [header file]
// https://www.loc.gov/preservation/digital/formats/fdd/fdd000422.shtml:
/*
The .flt file holds values for a single numeric measure, a value for each cell in the rectangular grid.  
The numeric values are in IEEE floating-point 32-bit (aka single-precision) signed binary format. 
The first number in the .flt file corresponds to the top left cell of the raster/grid. 
Going from left to right along the top row, the first 32 bits form the value for the first cell, 
the next 32 bits the value for the second cell, and so on, to the end of the top row. 
This is repeated for the second row of the raster, continuing to the lower right-hand cell.
*/
// https://anuga.anu.edu.au/ticket/211 [xllcorner and yllcorner]
// https://pubs.usgs.gov/tm/11b9/tm11B9.pdf [6-pixel tile overlap; 32-bit data]

// -----------  grid_float_tile ----------------

/*! \class  grid_float_tile
    \brief  Encapsulate a USGS GridFloat tile
*/

class grid_float_tile
{
  enum class QUADRANT { Q0,     ///< too close to the centre for the quadrant number to be meaningful
                        Q1,     ///< first quadrant
                        Q2,     ///< second quadrant
                        Q3,     ///< third quadrant
                        Q4      ///< fourth quadrant
                      };        ///< quadrants in a tile
  
protected:

  int _n_columns { 0 };         ///< number of columns in the tile
  int _n_rows    { 0 };         ///< number of rows in the tile
  
  double _xllcorner { 0 };      ///< x value of lower left corner
  double _yllcorner { 0 };      ///< y value of lower left corner
  
  double _cellsize { 0 };
  
  int _nodata_value { 0 };      ///< first NODATA value
  int _nodata       { 0 };      ///< second NODATA value  -- documentation isn't clear about whyt these both exist
  
  std::string _byte_order;      ///< MUST be "LSBFIRST"; currently unchecked
  
// assume float is 32-bit (this is checked before use)
  std::vector<std::vector<float>> _data;    ///< actual data in the tile; enables access as [latitude][longitude]
  
  mutable std::ifstream* _ifsp { nullptr };                       // small-memory data access; MUST use pointter as ifstreams are non-copyable
  bool                   _sm   { false };
  std::string            _data_filename;
  
  int _n_invalid_data { 0 };    ///< number of NODATA or NODATA_VALUE cells
  
  double _xl { 0 };             ///< longitude of western edge
  double _xr { 0 };             ///< longitude of eastern edge
  double _yb { 0 };             ///< latitude of southern edge
  double _yt { 0 };             ///< latitude of northern edge

/*! \brief          Is a value between two other values?
    \param  value   value to test
    \param  v1      one bound
    \param  v2      other bound
    \return         whether <i>value</i> is between <i>v1</i> and <i>v2</i>
    
    <i>v1</i> and <i>v2</i> may be in either order
*/  
 template<typename T> 
   const bool _is_between(const T& value, const T& v1, const T& v2) const
   { if ( (value >= v1) and (value <= v2) )
       return true;
       
     if ( (value >= v2) and (value <= v1) )
       return true;
       
     return false;
   }

/*! \brief              Map a latitude to a row number
    \param  latitude    latitude to map
    \return             row number that contains latitude <i>latitude</i>
*/  
  const int _map_latitude_to_index(const double& latitude) const;
  
/*! \brief              Map a longitude to a column number
    \param  longitude   longitude to map
    \return             column number that contains longitude <i>longitude</i>
*/ 
  const int _map_longitude_to_index(const double& longitude) const;
  
/*! \brief              Return the QUADRANT within a cell for a particular latitude and longitude
    \param  latitude    latitude of point
    \param  longitude   longitude of point
    \return             the quadrant within a tile cell in which the point lies
    
    Returns Q0 if the point is within one metre of the centre of the cell
*/
  const QUADRANT _quadrant(const double& latitude, const double& longitude) const;
  
public:

/*! \brief                      Constructor
    \param  header_filename     name of the header file
    \param  data_filename       name of the data file
*/
  grid_float_tile(const std::string& header_filename, const std::string& data_filename, const bool small_memory = false);

/// destructor
  inline virtual ~grid_float_tile(void)
  { if (_ifsp)
      delete _ifsp;
  }

/// Textual description of the tile
  const std::string to_string(void) const;

/*! \brief              Is a point within the tile?
    \param  latitude    latitude of point
    \param  longitude   longitude of point
    \return             whether the point is within the tile
*/  
  inline const bool is_in_tile(const double& latitude, const double& longitude) const
    { return (_is_between(latitude, _yb, _yt) and _is_between(longitude, _xl, _xr)); }

/*! \brief              The value of the cell that contains a point
    \param  latitude    latitude of point
    \param  longitude   longitude of point
    \return             the value of the cell containing the point
    
    Returns <i>_no_data</i> if the point is not within the tile
*/    
  const float cell_value(const double& latitude, const double& longitude) const;
  
/*! \brief      The value of the cell with particular indices
    \param  ip  index pair (latitude index, longitude index)
    \return     the value of the cell [ip.first][ip.second]
    
    Performs no bounds checking
*/
  const float cell_value(const std::pair<int, int>& ip) const;  // pair is lat index, long index
  
/*! \brief              The weighted average of the cells near a point
    \param  latitude    latitude of point
    \param  longitude   longitude of point
    \return             the interpolated value of nearby cells, to give the value for the point
    
    Returns <i>_no_data</i> if the point is not within the tile, or if there are insufficient data
    to return a valid response.
*/ 
  const float interpolated_value(const double& latitude, const double& longitude) const;

/*! \brief      The weighted average of the cells near a point
    \param  ll  latitude and longitude of point
    \return     the interpolated value of nearby cells, to give the value for the point
    
    Returns <i>_no_data</i> if the point is not within the tile
*/
  inline const float interpolated_value(const std::pair<double, double>& ll) const
    { return interpolated_value(ll.first, ll.second); }

 /*! \brief             Convert a latitude and longitude to the equivalent indices
     \param  latitude   latitude of point
     \param  longitude  longitude of point
     \return            indices equivalent to <i>latitude</i> and <i>longitude</i>
*/ 
  inline const std::pair<int, int> index_pair(const double& latitude, const double& longitude) const
    { return { _map_latitude_to_index(latitude), _map_longitude_to_index(longitude) }; }
    
/*! \brief          Get the central latitude and longitude for a cell identified by an index pair
    \param  ipair   index pair (latitude index, longitude index)
    \return         the latitude and longitude of the centre of the cell [ipair.first][ipair.second]
    
    Performs no bounds checking
*/
  const std::pair<double /* latitude */, double /* longitude */> cell_centre(const std::pair<int, int>& ipair) const;
  
/*! \brief          Get the central latitude and longitude for a cell identified by an index pair
    \param  ipair   index pair (latitude index, longitude index)
    \return         the latitude and longitude of the centre of the cell [ipair.first][ipair.second]
*/
  inline const std::pair<double, double> cell_centre(const std::pair<double, double>& ipair) const
    { return cell_centre(index_pair(ipair.first, ipair.second)); }

/*! \brief      Get the central latitude and longitude for a cell containing a particular latitude and longitude
    \param      latitude   latitude of point
    \param      longitude  longitude of point    \param  ipair   index pair (latitude index, longitude index)
    \return     the latitude and longitude of the centre of the cell that contains [latitude, longitude]
*/
  inline const std::pair<double, double> cell_centre(const double& latitude, const double& longitude) const
    { return cell_centre(index_pair(latitude, longitude)); }
   
  inline const bool valid_height(const float& h) const
    { return ( h > (_nodata + 1) ); }
};

/*! \brief          Obtain distance in km between two locations
    \param  lat1    latitude of source, in degrees (+ve north)
    \param  long1   longitude of source, in degrees (+ve east)
    \param  lat2    latitude of target, in degrees (+ve north)
    \param  long2   longitude of target, in degrees (+ve east)
    \return         distance between source and target, in km

    See http://www.movable-type.co.uk/scripts/latlong.html:

    a = sin²(Δφ/2) + cos(φ1).cos(φ2).sin²(Δλ/2)
    c = 2.atan2(√a, √(1−a))
    d = R.c
    where   φ is latitude, λ is longitude, R is earth’s radius (mean radius = 6,371km)

    θ = atan2( sin(Δλ).cos(φ2), cos(φ1).sin(φ2) − sin(φ1).cos(φ2).cos(Δλ) )
*/
const double distance(const double& lat1, const double& long1, const double& lat2, const double& long2);

inline const double distance(const std::pair<double, double>& lat_long_1, const std::pair<double, double>& lat_long_2)
  { return distance(lat_long_1.first, lat_long_1.second, lat_long_2.first, lat_long_2.second); }

inline const double distance(const std::pair<double, double>& lat_long_1, const double& lat2, const double& long2)
  { return distance(lat_long_1.first, lat_long_1.second, lat2, long2); }

inline const double distance(const double& lat1, const double& long1, const std::pair<double, double>& lat_long_2)
  { return distance(lat1, long1, lat_long_2.first, lat_long_2.second); }

const std::pair<double, double> ll_from_bd(const double& lat1 /* deg */, const double& long1 /* deg */ , const double& bearing_d /* degrees */, const double& distance_m /* metres */);

inline const std::pair<double, double> ll_from_bd(const std::pair<double, double>& ll, const double& bearing_d /* degrees */, const double& distance_m /* metres */)
  { return ll_from_bd(ll.first, ll.second, bearing_d, distance_m); }

const double bearing(const int delta_x, const int delta_y);  // bearing in degrees

/*! \brief              Return a base filename derived from latitude and longitude
    \param  latitude    latitude
    \param  longitude   longitude
    \return             the base name of the file that contains the data for the tile that contains the point at <i>latitude</i>, <i>longitude</i>
    
    "nLLwLLL"
*/
const std::string base_filename(const double& latitude, const double& longitude);

// "nLLwLLL"
inline const std::string base_filename(const std::pair<double, double>& ll)
  { return base_filename(ll.first, ll.second); }
  
/*! \brief              Return a base filename derived from latitude and longitude
    \param  llcode      the llcode [lat * 1000 + (+ve)long]
    \return             the base name of the file that contains the data for the tile that contains the point at <i>latitude</i>, <i>longitude</i>
    
    "nLLwLLL"
*/
const std::string base_filename(const int llcode);
 
const std::string local_tile_filename(const double& latitude, const double& longitude, const std::string& directory);

inline const std::string local_tile_filename(const std::pair<double, double>& ll, const std::string& directory)
  { return local_tile_filename(ll.first, ll.second, directory); }

const std::string local_tile_filename(const int llcode, const std::string& directory);

const std::string remote_tile_filename(const double& latitude, const double& longitude);

inline const std::string remote_tile_filename(const std::pair<double, double>& ll)
  { return remote_tile_filename(ll.first, ll.second); }

/*! \brief            Get the remote filename corresponding to a particular tile
    \param  llcode    the llcode [lat * 1000 + (+ve)long]
    \return           the filename on the USGS server that corresponds to the tile that contains the point at at the <i>llcode</i> locatioin
    
    A typical filename is: "USGS_NED_13_n41w106_GridFloat.zip"
*/
inline const std::string remote_tile_filename(const int llcode)
  { return ("USGS_NED_13_"s + base_filename(llcode) + "_GridFloat.zip"s); }

/*! \brief              Get the remote filename corresponding to a particular tile
    \param  latitude    latitude
    \param  longitude   longitude
    \return             the filename on the USGS server that corresponds to the tile that contains the point at <i>latitude</i>, <i>longitude</i>
    
    A typical filename is: "USGS_NED_13_n41w106_GridFloat.zip"
*/
inline const std::string remote_tile_filename(const double& latitude, const double& longitude)
  { return ("USGS_NED_13_"s + base_filename(latitude, longitude) + "_GridFloat.zip"s); }

/*! \brief              Get the local filename corresponding to the header information for particular tile
    \param  latitude    latitude
    \param  longitude   longitude
    \param  directory   the local directory
    \return             the local filename that contains the header information for the tile that contains the point at <i>latitude</i>, <i>longitude</i>
*/
const std::string local_header_filename(const double& latitude, const double& longitude, const std::string& directory);

/*! \brief              Get the local filename corresponding to the header information for particular tile
    \param  ll          latitude and longitude of point
    \param  directory   the local directory
    \return             the local filename that contains the header information for the tile that contains the point at <i>ll</i>
*/
inline const std::string local_header_filename(const std::pair<double, double>& ll, const std::string& directory)
  { return local_header_filename(ll.first, ll.second, directory); }
  
/*! \brief              Get the local filename corresponding to the header information for particular tile
    \param  llcode      the llcode [lat * 1000 + (+ve)long]
    \param  directory   the local directory
    \return             the local filename that contains the header information for the tile that contains the point at <i>llcode</i>
*/
inline const std::string local_header_filename(const int llcode, const std::string& directory)
  { return (dirname_with_slash(directory) + "usgs_ned_13_"s + base_filename(llcode) + "_gridfloat.hdr"s); }

/*! \brief              Get the local filename corresponding to the data for particular a tile
    \param  latitude    latitude
    \param  longitude   longitude
    \param  directory   the local directory
    \return             the local filename that contains the data for the tile that contains the point at <i>latitude</i>, <i>longitude</i>
*/
const std::string local_data_filename(const double& latitude, const double& longitude, const std::string& directory);

inline const std::string local_data_filename(const std::pair<double, double>& ll, const std::string& directory)
  { return local_data_filename(ll.first, ll.second, directory); }

/*! \brief              Get the local filename corresponding to the data for a particular tile
    \param  llcode      the llcode [lat * 1000 + (+ve)long]
    \param  directory   the local directory
    \return             the local filename that contains the data for the tile that contains the point at <i>llcode</i>
*/
inline const std::string local_data_filename(const int llcode, const std::string& directory)
  { return (dirname_with_slash(directory) + "usgs_ned_13_"s + base_filename(llcode) + "_gridfloat.flt"s); }

// lambdas can't be overloaded! lat-long-code
inline const int llc(const double& latitude, const double& longitude)
  { return ( int(latitude + 1) * 1000 + int(-(longitude - 1) ) ); } 

inline const int llc(const std::pair<double, double>& ll) 
  { return ( llc(ll.first, ll.second) ); };

inline const int llc(const std::string& basefilename) //"nLLwLLL"
  { return ( from_string<int>(basefilename.substr(1, 2)) * 1000 + from_string<int>(basefilename.substr(4, 3))); }

//inline const double curvature_correction(const double& d)
//  { return ( ( (1 / cos(d / RE) ) - 1) * RE ); }

inline const double curvature_correction(const double& d)
  { return ( ( 1 - cos(d / RE) ) * RE ); }

class grid_float_error : public x_error
{
protected:
  
public:

/*!	\brief	    Construct from error code and reason
	\param	n	error code
	\param	s	reason
*/
  grid_float_error(const int n, const std::string& s) :
    x_error(n, s)
  { }
};

#endif    // GRID_FLOAT_H
