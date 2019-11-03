// $Id: grid_float.cpp 15 2019-11-03 15:02:05Z n7dr $

// Released under the GNU Public License, version 2

// Principal author: N7DR

// Copyright owners:
//    N7DR

#include "diskfile.h"
#include "grid_float.h"
#include "string_functions.h"

//#include <cmath>
#include <iostream>
#include <iterator>
#include <streambuf>

using namespace std;

extern bool debug;

//https://prd-tnm.s3.amazonaws.com/StagedProducts/Elevation/13/GridFloat/n44w109.zip

//command = ***wget -q -O /tmp/drpattern/n44w109.zip https://prd-tnm.s3.amazonaws.com/StagedProducts/Elevation/13/GridFloat/USGS_NED_13_n44w109_GridFloat.zip***

/*! \brief                      Download a tile from the USGS if we don't already have it
    \param  llc                 the llcode [lat * 1000 + (+ve)long]
    \param  local_directory     the local directory containing USGS files
*/ 
void download_if_necessary(const int llc, const string& local_directory)
{ bool need_to_download { false };

  const string local_dirname    { local_directory + ( (last_char(local_directory) == '/') ? ""s : "/"s ) };       // ensure a terminating slash
  const string full_header_name { local_dirname + "usgs_ned_13_" + base_filename(llc) + "_gridfloat.hdr"s };    // full name of local header file
  const string full_data_name   { local_dirname + "usgs_ned_13_" + base_filename(llc) + "_gridfloat.flt"s };    // full name of local data file

  if (!file_exists(full_header_name) or file_empty(full_header_name) or !file_exists(full_data_name) or file_empty(full_data_name))
    need_to_download = true;                                                                                    // don't need to download if the header and data files are present
    
  if (!need_to_download)
    return;                                                                                                     // we're done here

  directory_create_if_necessary(local_directory);

  const string remote_directory { R"(https://prd-tnm.s3.amazonaws.com/StagedProducts/Elevation/13/GridFloat/)" };  

  vector<string> remote_filenames;      // remote filenames to try, in order
  
  remote_filenames.push_back("USGS_NED_13_"s + base_filename(llc) + "_GridFloat.zip"s);
  remote_filenames.push_back(base_filename(llc) + ".zip");
  
  const string local_filename { local_dirname + base_filename(llc) + ".zip"s };
  
  bool downloaded { false };
  
  string command;
  
  for (unsigned int n = 0; !downloaded and n < remote_filenames.size(); ++n)
  { if (!file_exists(local_filename))                                          // don't download if it already exists
    { cout << "File " << local_filename << " does not exist; download attempt number " << (n + 1) << endl;
  
      command = R"(wget -q -O )"  + local_filename + " "s + remote_directory + remote_filenames[n];

      if (debug)
        cout << "command = ***" << command << "***" << endl;
      
      system(command.c_str());
    }
    
    if (file_empty(local_filename))
      file_delete(local_filename);
    else
      downloaded = true;      
  }

  cout << (downloaded ? "Download succeeded" : "Download did not succeed") << endl;
  
  if (!downloaded)
  { cerr << "download failed; exiting" << endl;
    exit(-1);
  }
  
// we get here only if the download succeeded
  const string default_header_name { "usgs_ned_13_" + base_filename(llc) + "_gridfloat.hdr"s };

  command = "unzip -qq -o -d "s + local_directory + " "s + local_filename + " " + default_header_name;  // overwrite!!
  
  system(command.c_str());
  
  if (!file_exists(local_dirname + default_header_name))
  { if (debug)
      cout << "Header file " << local_dirname + default_header_name << " does not exist; trying alternative name" << endl;
  
    const string alternative_header_name { "float" + base_filename(llc) + "_13.hdr" };
    
    command = "unzip -qq -o -d "s + local_directory + " "s + local_filename + " " + alternative_header_name;  // overwrite!!

    system(command.c_str());

    if (!file_exists(local_dirname + alternative_header_name))
    { cerr << "Alternative header file " << local_dirname + alternative_header_name << " does not exist; exiting" << endl;
      exit(-1);
    }
    
    command = "mv "s + local_dirname + alternative_header_name + " "s + local_dirname + default_header_name;
    system(command.c_str());
  }
  
  const string default_data_name { "usgs_ned_13_"s + base_filename(llc) + "_gridfloat.flt"s };

  command = "unzip -qq -o -d "s + local_directory + " "s + local_filename + " " + default_data_name;  // overwrite!!
  
  system(command.c_str());
  
  if (!file_exists(local_dirname + default_data_name))
  { if (debug)
      cout << "Data file " << local_dirname + default_data_name << " does not exist; trying alternative name" << endl;
  
    const string alternative_data_name { "float"s + base_filename(llc) + "_13.flt"s };
    
    command = "unzip -qq -o -d "s + local_directory + " "s + local_filename + " " + alternative_data_name;  // overwrite!!

    system(command.c_str());

    if (!file_exists(local_dirname + alternative_data_name))
    { cerr << "Alternative data file " << local_dirname + alternative_data_name << " does not exist; exiting" << endl;
      exit(-1);
    }
    
    command = "mv "s + local_dirname + alternative_data_name + " "s + local_dirname + default_data_name;
    system(command.c_str());
  }
}

/*! \brief              Map a latitude to a row number
    \param  latitude    latitude to map
    \return             row number that contains latitude <i>latitude</i>
*/ 
const int grid_float_tile::_map_latitude_to_index(const double& latitude) const
{ const auto diff    { _yt - latitude };
  const auto n_cells { diff / _cellsize };
  
  return int(n_cells);  
}
  
/*! \brief              Map a longitude to a column number
    \param  longitude   longitude to map
    \return             column number that contains longitude <i>longitude</i>
*/
const int grid_float_tile::_map_longitude_to_index(const double& longitude) const
{ const auto diff    { longitude - _xl };
  const auto n_cells { diff / _cellsize };
  
  return int(n_cells);
}

/*! \brief              Return the QUADRANT within a cell for a particular latitude and longitude
    \param  latitude    latitude of point
    \param  longitude   longitude of point
    \return             the quadrant within a tile cell in which the point lies
    
    Returns Q0 if the point is within one metre of the centre of the cell
*/
const grid_float_tile::QUADRANT grid_float_tile::_quadrant(const double& latitude, const double& longitude) const
{ const auto    centre      { cell_centre(latitude, longitude) };
  const double& c_latitude  { centre.first };
  const double& c_longitude { centre.second };
  const double  dist        { distance(centre, latitude, longitude) };
  
// arbitrary, if distance < 1m, then treat as 0
  if (dist < 1)
    return QUADRANT::Q0;
  
  if ( (latitude >= c_latitude) and (longitude >= c_longitude) )
    return QUADRANT::Q1;
    
  if ( (latitude >= c_latitude) and (longitude <= c_longitude) )
    return QUADRANT::Q2;
    
  if ( (latitude <= c_latitude) and (longitude <= c_longitude) )
    return QUADRANT::Q3;
    
  return QUADRANT::Q4;
}

/*! \brief                      Constructor
    \param  header_filename     name of the header file
    \param  data_filename       name of the data file
*/
grid_float_tile::grid_float_tile(const std::string& header_filename, const std::string& data_filename, const bool small_memory) :
  _data_filename(data_filename),
  _sm(small_memory)
{ if (debug)
    cout << "data_filename = " << data_filename << endl;
  
  if (sizeof(float) != 4)
  { cerr << "ERROR: size of float is " << sizeof(float) << ", not 4" << endl;
    exit(-1);
  }
  
  if (!file_exists(header_filename))
  { cerr << "ERROR: header file " << header_filename << " does not exist" << endl;
    exit(-1);
  }
  
  if (!file_exists(data_filename))
  { cerr << "ERROR: data file " << data_filename << " does not exist" << endl;
    exit(-1);
  }
  
// import the header data
  { const vector<string> header_lines { to_lines(squash(to_upper(remove_char(read_file(header_filename), CR_CHAR)))) };
  
    for (const string& line : header_lines)
    { const vector<string> fields { split_string(line, " "s) };
  
      if (fields.size() != 2)
      { cerr << "ERROR in line in header file: " << line << " in file: " << header_filename << endl;
        exit(-1);
      }
    
      if (fields[0] == "NCOLS"s)
        _n_columns = from_string<decltype(_n_columns)>(fields[1]);
      
      if (fields[0] == "NROWS"s)
        _n_rows = from_string<decltype(_n_rows)>(fields[1]);
      
      if (fields[0] == "XLLCORNER"s)
        _xllcorner = from_string<decltype(_xllcorner)>(fields[1]);

      if (fields[0] == "YLLCORNER"s)
        _yllcorner = from_string<decltype(_yllcorner)>(fields[1]);
      
      if (fields[0] == "CELLSIZE"s)
        _cellsize = from_string<decltype(_cellsize)>(fields[1]);
            
      if (fields[0] == "NODATA_VALUE"s)
        _nodata_value = from_string<decltype(_nodata_value)>(fields[1]);
  
      if (fields[0] == "NODATA"s)
        _nodata = from_string<decltype(_nodata)>(fields[1]);

      if (fields[0] == "BYTEORDER"s)
        _byte_order = fields[1];     
    } 
    
// other values
    _xl = _xllcorner;
    _xr = _xllcorner + _cellsize * _n_columns;
    
    _yb = _yllcorner;
    _yt = _yllcorner + _cellsize * _n_rows;
  }
  
// set the capacity of the data vectors
  if (!small_memory)
  { _data.reserve(_n_rows);

// import the elevation data
    { ifstream ifs { data_filename };
  
      const int row_length { static_cast<int>(sizeof(float) * _n_columns) };
    
      for (int n = 0; n < _n_rows; ++n)
      { vector<float> row(_n_columns);
    
        char* cp { reinterpret_cast<char*>(row.data()) };
      
        ifs.read (cp, row_length);
      
        _data.push_back(row);
      }
    }                             // finished importing data
  
// count the bad data
    for (int n1 = 0; n1 < _n_rows; ++n1)
    { const auto& row { _data[n1] };
  
      for (int n2 = 0; n2 < _n_columns; ++ n2)
        if (row[n2] < (_nodata + 1))
          _n_invalid_data++;
    }
    
    if (debug)  
      cout << "Number of invalid data elements = " << comma_separated_string(_n_invalid_data) << " out of " << comma_separated_string(_n_rows * _n_columns) << endl;
  }
  else    // small memory
  { _ifsp = new ifstream(data_filename);
  
    if (! _ifsp -> good())
    { cerr << "ERROR IFSTREAM IN BAD STATE" << endl;
      exit(-1);
    }
    
// count the bad data

    float value;
    long counter { 0 };
    
    while (! _ifsp -> eof())
    { _ifsp -> read(reinterpret_cast<char*>(&value), sizeof(value));
      
      if (! _ifsp ->eof())
      { counter++;
    
        if (value < (_nodata + 1))
          _n_invalid_data++;
      }
    }

    if (debug)    
      cout << "Number of invalid data elements [sm] = " << comma_separated_string(_n_invalid_data) << " out of " << comma_separated_string(counter) << endl;

    delete(_ifsp);
    _ifsp = nullptr;    
  }
}

/// Textual description of the tile
const string grid_float_tile::to_string(void) const
{ string rv;

  rv += "Number of columns      = "s + ::to_string(_n_columns) + EOL;
  rv += "Number of rows         = "s + ::to_string(_n_rows) + EOL;
  rv += "XLLCORNER              = "s + ::to_string(_xllcorner) + EOL;
  rv += "YLLCORNER              = "s + ::to_string(_yllcorner) + EOL;
  rv += "Cell size              = "s + ::to_string(_cellsize) + EOL;
  rv += "NODATA_value           = "s + ::to_string(_nodata_value) + EOL;
  rv += "NODATA                 = "s + ::to_string(_nodata) + EOL;
  rv += "Byte order             = "s + _byte_order + EOL;
  
  rv += "Left X                 = "s + ::to_string(_xl) + EOL;
  rv += "Right X                = "s + ::to_string(_xr) + EOL;
  rv += "Bottom Y               = "s + ::to_string(_yb) + EOL;
  rv += "Top Y                  = "s + ::to_string(_yt) + EOL;

  rv += "Number of invalid data = "s + ::to_string(_n_invalid_data);
  
  return rv;
}

/*! \brief              The value of the cell that contains a point
    \param  latitude    latitude of point
    \param  longitude   longitude of point
    \return             the value of the cell containing the point
    
    Returns <i>_no_data</i> if the point is not within the tile
*/ 
const float grid_float_tile::cell_value(const double& latitude, const double& longitude) const
{ if (is_in_tile(latitude, longitude))
  { const int row_nr    { _map_latitude_to_index(latitude) };
    const int column_nr { _map_longitude_to_index(longitude) };    
    const int row_size  { _n_columns * 4 };
    
    if (_sm)
    { if (_ifsp == nullptr)
        _ifsp = new ifstream(_data_filename);
      
      long posn = (row_nr * row_size) + (column_nr * 4);
    
      _ifsp -> seekg (0, _ifsp -> end);
      long length = _ifsp -> tellg();
    
      _ifsp ->seekg( (row_nr * row_size) + column_nr * 4);
    
      if (! _ifsp -> good())
      { cerr << "ERROR IFSTREAM IN BAD STATE #1 IN CELL_VALUE WHEN SEEKING TO " << posn << ", length = " << length << endl;
        exit(-1);
      }
    
      float value;
      _ifsp -> read(reinterpret_cast<char*>(&value), sizeof(value));
      
      if (! _ifsp -> good())
      { cerr << "ERROR IFSTREAM IN BAD STATE #2 IN CELL_VALUE" << endl;
        exit(-1);
      }

      return value;
    } 
    else
      return _data[row_nr][column_nr];
  }
  else
    return _nodata;
}

/*! \brief      The value of the cell with particular indices
    \param  ip  index pair (latitude index, longitude index)
    \return     the value of the cell [ip.first][ip.second]
    
    Performs no bounds checking
*/
const float grid_float_tile::cell_value(const std::pair<int, int>& ip) const  // pair is lat index, long index
{ if (_sm)
  { if (_ifsp == nullptr)
      _ifsp = new std::ifstream(_data_filename);
      
    const int  row_size { _n_columns * 4 };      // in bytes
    const long posn     { (ip.first * row_size) + (ip.second * 4) };    // in bytes
    
    _ifsp ->seekg(posn);
    
    if (! _ifsp -> good())
    { cerr << "ERROR IFSTREAM IN BAD STATE #1 IN CELL_VALUE WHEN SEEKING TO " << posn << endl;
      exit(-1);
    }
    
    float value;
    
    _ifsp -> read(reinterpret_cast<char*>(&value), sizeof(value));
      
    if (! _ifsp -> good())
    { cerr << "ERROR IFSTREAM IN BAD STATE #2 IN CELL_VALUE" << endl;
      exit(-1);
    }

    return value;      
  }
  else
    return _data[ip.first][ip.second]; 
}

/*! \brief          The latitude and longitude of the cell with particular indices
    \param  ipair   index pair (latitude index, longitude index)
    \return         the latitude and longitude of the centre of the cell [ipair.first][ipair.second]
    
    Performs no bounds checking
*/
const pair<double, double> grid_float_tile::cell_centre(const pair<int, int>& ipair) const
{ const double lat_0     { _yt - _cellsize / 2 };
  const double latitude  { lat_0 - ipair.first * _cellsize };
  const double long_0    { _xl + _cellsize / 2 };
  const double longitude { long_0 + ipair.second * _cellsize };
  
  return { latitude, longitude };
} 

/*! \brief              The weighted average of the cells near a point
    \param  latitude    latitude of point
    \param  latitude    latitude of point
    \param  longitude   longitude of point
    \return             the interpolated value of nearby cells, to give the value for the point
    
    Returns <i>_no_data</i> if the point is not within the tile, or if there are insufficient data
    to return a valid response.
*/
const float grid_float_tile::interpolated_value(const double& latitude, const double& longitude) const
{ const pair<double, double> ll_centre { cell_centre(latitude, longitude) };
  const QUADRANT             q         { _quadrant(latitude, longitude) };
  
  switch (q)
  { case QUADRANT::Q0 :
    { const auto cv { cell_value(latitude, longitude) };
      
      if (cv < -9000)
        throw grid_float_error(GRID_FLOAT_NODATA, ( "Q0: Insufficient data when interpolating at "s + ::to_string(latitude) + ", "s + ::to_string(longitude)) );
      
      return cv;
    }
      
    case QUADRANT::Q1 :
    { array<float, 4> heights;
      array<double, 4> distances;
      
      const pair<int, int> indices_0 { index_pair(latitude, longitude) };
      
      heights[0] = cell_value(latitude, longitude);
      distances[0] = distance(ll_centre, latitude, longitude);
      
// move right => increment second index
      const pair<int, int> indices_1 { indices_0.first, indices_0.second + 1 };
      
      const pair<double, double> ll_centre_1 { cell_centre(indices_1) };
      
      heights[1] = cell_value(indices_1);
      distances[1] = distance(ll_centre_1, latitude, longitude);

// move up => decrement first index            
      pair<int, int> indices_2 { indices_0.first - 1, indices_0.second };
      
      const pair<double, double> ll_centre_2 { cell_centre(indices_2) };
      
      heights[2] = cell_value(indices_2);
      distances[2] = distance(ll_centre_2, latitude, longitude);

// move up and to the right => decrement first index, increment second index            
      pair<int, int> indices_3 { indices_0.first - 1, indices_0.second + 1 };
      
      const pair<double, double> ll_centre_3 { cell_centre(indices_3) };
      
      heights[3] = cell_value(indices_3);
      distances[3] = distance(ll_centre_3, latitude, longitude);

// calculate the weighted mean
      double h     { 0 };
      double inv_d { 0 };
      
      for (auto n = 0; n < 4; ++n)
      { if (valid_height(heights[n]))
        { h += heights[n] / distances[n];
          inv_d += 1 / distances[n];
        }
      }
      
      if (inv_d == 0)
        throw grid_float_error(GRID_FLOAT_NODATA, ( "Q1: Insufficient data when interpolating at "s + ::to_string(latitude) + ", " + ::to_string(longitude)) );
      
      const double rv { (inv_d == 0) ? _nodata : (h / inv_d) };
      
      return rv;
    }
    
    case QUADRANT::Q2 :
    { array<float, 4> heights;
      array<double, 4> distances;
      
      const pair<int, int> indices_0 { index_pair(latitude, longitude) };
      
      heights[0] = cell_value(latitude, longitude);
      distances[0] = distance(ll_centre, latitude, longitude);
      
// move left => decrement second index
      const pair<int, int>       indices_1   { indices_0.first, indices_0.second - 1 };
      const pair<double, double> ll_centre_1 { cell_centre(indices_1) };
      
      heights[1] = cell_value(indices_1);
      distances[1] = distance(ll_centre_1, latitude, longitude);

// move up => decrement first index            
      const pair<int, int>       indices_2   { indices_0.first - 1, indices_0.second };
      const pair<double, double> ll_centre_2 { cell_centre(indices_2) };
      
      heights[2] = cell_value(indices_2);
      distances[2] = distance(ll_centre_2, latitude, longitude);

// move up and to the left => decrement first index, decrement second index            
      const pair<int, int>       indices_3   { indices_0.first - 1, indices_0.second - 1 };
      const pair<double, double> ll_centre_3 { cell_centre(indices_3) };
      
      heights[3] = cell_value(indices_3);
      distances[3] = distance(ll_centre_3, latitude, longitude);

// calculate the weighted mean
      double h     { 0 };
      double inv_d { 0 };
      
      for (auto n = 0; n < 4; ++n)
      { h += heights[n] / distances[n];
        inv_d += 1 / distances[n];
      }
      
      if (inv_d == 0)
        throw grid_float_error(GRID_FLOAT_NODATA, ( "Q2: Insufficient data when interpolating at "s + ::to_string(latitude) + ", " + ::to_string(longitude)) );

      const double rv { h / inv_d };
      
      return rv;
    }

    case QUADRANT::Q3 :
    { array<float, 4> heights;
      array<double, 4> distances;
      
      const pair<int, int> indices_0 { index_pair(latitude, longitude) };
      
      heights[0] = cell_value(latitude, longitude);
      distances[0] = distance(ll_centre, latitude, longitude);
      
// move left => decrement second index
      const pair<int, int>       indices_1   { indices_0.first, indices_0.second - 1 };
      const pair<double, double> ll_centre_1 { cell_centre(indices_1) };
      
      heights[1] = cell_value(indices_1);
      distances[1] = distance(ll_centre_1, latitude, longitude);

// move down => increment first index            
      const pair<int, int>       indices_2   { indices_0.first + 1, indices_0.second };
      const pair<double, double> ll_centre_2 { cell_centre(indices_2) };
      
      heights[2] = cell_value(indices_2);
      distances[2] = distance(ll_centre_2, latitude, longitude);

// move down and to the left => increment first index, decrement second index            
      const pair<int, int>       indices_3   { indices_0.first + 1, indices_0.second - 1 };
      const pair<double, double> ll_centre_3 { cell_centre(indices_3) };
      
      heights[3] = cell_value(indices_3);
      distances[3] = distance(ll_centre_3, latitude, longitude);

// calculate the weighted mean
      double h     { 0 };
      double inv_d { 0 };
      
      for (auto n = 0; n < 4; ++n)
      { h += heights[n] / distances[n];
        inv_d += 1 / distances[n];
      }

      if (inv_d == 0)
        throw grid_float_error(GRID_FLOAT_NODATA, ( "Q3: Insufficient data when interpolating at "s + ::to_string(latitude) + ", " + ::to_string(longitude)) );
      
      const double rv { h / inv_d };
      
      return rv;
    }

    case QUADRANT::Q4 :
    { array<float, 4> heights;
      array<double, 4> distances;
      
      const pair<int, int> indices_0 { index_pair(latitude, longitude) };
      
      heights[0] = cell_value(latitude, longitude);
      distances[0] = distance(ll_centre, latitude, longitude);
      
// move right => increment second index
      const pair<int, int>       indices_1   { indices_0.first, indices_0.second + 1 };
      const pair<double, double> ll_centre_1 { cell_centre(indices_1) };
      
      heights[1] = cell_value(indices_1);
      distances[1] = distance(ll_centre_1, latitude, longitude);

// move down => increment first index            
      const pair<int, int>       indices_2   { indices_0.first + 1, indices_0.second };
      const pair<double, double> ll_centre_2 { cell_centre(indices_2) };
      
      heights[2] = cell_value(indices_2);
      distances[2] = distance(ll_centre_2, latitude, longitude);

// move down and to the right => increment first index, increment second index            
      const pair<int, int>       indices_3   { indices_0.first + 1, indices_0.second + 1 };
      const pair<double, double> ll_centre_3 { cell_centre(indices_3) };
      
      heights[3] = cell_value(indices_3);
      distances[3] = distance(ll_centre_3, latitude, longitude);

// calculate the weighted mean
      double h     { 0 };
      double inv_d { 0 };
      
      for (auto n = 0; n < 4; ++n)
      { h += heights[n] / distances[n];
        inv_d += 1 / distances[n];
      }

      if (inv_d == 0)
        throw grid_float_error(GRID_FLOAT_NODATA, ( "Q4: Insufficient data when interpolating at "s + ::to_string(latitude) + ", " + ::to_string(longitude)) );
      
      const double rv { h / inv_d };
      
      return rv;
    }
  }
  
  return _nodata;    // just to keep the compiler happy
}

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
const double distance(const double& lat1, const double& long1, const double& lat2, const double& long2)
{ const double delta_phi   { lat2 - lat1 };
  const double delta_phi_2 { delta_phi / 2 };

  const double delta_lambda   { long2 - long1 };
  const double delta_lambda_2 { delta_lambda / 2 };

  const double a { sin(delta_phi_2 * DTOR) * sin(delta_phi_2 * DTOR) +
                   cos(lat1 * DTOR) * cos(lat2 * DTOR) * sin(delta_lambda_2 * DTOR) * sin(delta_lambda_2 * DTOR) 
                 };

  const double c { 2 * atan2(sqrt(a), sqrt(1 - a)) };
  const double d { RE * c };

  return d;
}

/*! \brief              Obtain latitude and longitude corresponding to a bearing and distance from a point
    \param  lat1        latitude of source, in degrees (+ve north)
    \param  long1       longitude of source, in degrees (+ve east)
    \param  bearing_d   bearing in degrees from source
    \param  distance_m  distance in metres (along Earth's surface) from source
    \return             latitude and longitude of target

Formula: φ2 = asin( sin φ1 ⋅ cos δ + cos φ1 ⋅ sin δ ⋅ cos θ )
	λ2 = λ1 + atan2( sin θ ⋅ sin δ ⋅ cos φ1, cos δ − sin φ1 ⋅ sin φ2 )
where 	φ is latitude, λ is longitude, θ is the bearing (clockwise from north), δ is the angular distance d/R; d being the distance travelled, R the earth’s radius
JavaScript:
(all angles
in radians)
	
var φ2 = Math.asin( Math.sin(φ1)*Math.cos(d/R) +
                    Math.cos(φ1)*Math.sin(d/R)*Math.cos(brng) );
var λ2 = λ1 + Math.atan2(Math.sin(brng)*Math.sin(d/R)*Math.cos(φ1),
                         Math.cos(d/R)-Math.sin(φ1)*Math.sin(φ2));

	The longitude can be normalised to −180…+180 using (lon+540)%360-180
Excel:
(all angles
in radians)
	lat2: =ASIN(SIN(lat1)*COS(d/R) + COS(lat1)*SIN(d/R)*COS(brng))
lon2: =lon1 + ATAN2(COS(d/R)-SIN(lat1)*SIN(lat2), SIN(brng)*SIN(d/R)*COS(lat1)) 
*/
const pair<double, double> ll_from_bd(const double& lat1 /* deg */, const double& long1 /* deg */ , const double& bearing_d /* degrees clockwise from north */, const double& distance_m /* metres */)
{ const double delta   { distance_m / RE };
  const double lat1_r  { lat1 * DTOR };
  const double long1_r { long1 * DTOR };
  const double theta   { bearing_d * DTOR };
  const double lat2_r  { asin ( sin (lat1_r * cos(delta) + cos(lat1_r) * sin(delta) * cos(theta)) ) };
  const double long2_r { long1_r + atan2( sin(theta) * sin(delta) * cos(lat1_r), cos(delta) - sin(lat1_r) * sin(lat2_r)) };
  const double lat2_d  { lat2_r * RTOD };
  const double long2_d { long2_r * RTOD };
  
  return { lat2_d, long2_d };
}

/*! \brief              Obtain the bearing (from north) associated with displacement by an amount horizontally and vertically
    \param  delta_x     number and direction of horizontal units
    \param  delta_y     number and direction of vertical units
    \return             the bearing, in degrees, associated with displacement by <i>delta_x</i> and <i>delta_y</i>
    
    "bearing" here means the initial bearing at which one must leave the central point; it is also the bearing at which a cell
    corresponding to <i>delta_x</i> and <i>delta_y</i> is to be plotted
*/
const double bearing(const int delta_x, const int delta_y)  // bearing in degrees
{ if ( (delta_x == 0) and (delta_y == 0) )
    return 0;
    
  if (delta_x == 0)
  { if (delta_y > 0)
      return 0;
    return 180;
  }
  
  if (delta_y == 0)
  { if (delta_x < 0)
      return 270;
    return 90;
  }
  
  if ( (delta_x > 0) and (delta_y > 0) )    // 0 -- 90
  { if (delta_y > delta_x)                  // 0 -- 45
      return atan((float)delta_x / (float)delta_y) * RTOD;
    return 90 - ( atan((float)delta_y / (float)delta_x) * RTOD );   // 45 -- 90
  } 
  
  if ( (delta_x > 0) and (delta_y < 0) )    // 90 -- 180
  { if (abs(delta_x) > abs(delta_y))         // 90 -- 135
      return 90 + ( atan((float)abs(delta_y) / (float)abs(delta_x)) * RTOD );
    return 180 - ( atan((float)abs(delta_x) / (float)abs(delta_y)) * RTOD );    // 135 -- 180
  }
  
  if ( (delta_x < 0) and (delta_y < 0) )    // 180 -- 270
  { if (abs(delta_x) < abs(delta_y))         // 180 -- 225
      return 180 + ( atan((float)abs(delta_x) / (float)abs(delta_y)) * RTOD );
    return 270 - ( atan((float)abs(delta_y) / (float)abs(delta_x)) * RTOD );    // 225 -- 270
  }
  
// only 270 -- 0 remain
  if (abs(delta_x) > abs(delta_y))         // 270 -- 315
      return 270 + ( atan((float)abs(delta_y) / (float)abs(delta_x)) * RTOD );
  return 360 - ( atan((float)abs(delta_x) / (float)abs(delta_y)) * RTOD );  // 315 -- 360
}

/*! \brief              Get the local filename corresponding to a particular tile
    \param  latitude    latitude
    \param  longitude   longitude
    \param  directory   the local directory
    \return             the local filename corresponding to the tile that contains the point at <i>latitude</i>, <i>longitude</i>
*/
const string local_tile_filename(const double& latitude, const double& longitude, const string& directory)
{ const string fn       { base_filename(latitude, longitude) + ".zip"s };
  
  return (dirname_with_slash(directory) + fn);
}

/*! \brief              Get the local filename corresponding to a particular tile
    \param  llcode      the llcode [lat * 1000 + (+ve)long] 
    \param  longitude   longitude
    \param  directory   the local directory
    \return             the local filename corresponding to the tile that contains the point at <i>latitude</i>, <i>longitude</i>
*/
const string local_tile_filename(const int llcode, const string& directory)
{ const string fn       { base_filename(llcode) + ".zip"s };
  
  return (dirname_with_slash(directory) + fn); 
}

/*! \brief              Get the local filename corresponding to the header information for a particular tile
    \param  latitude    latitude
    \param  longitude   longitude
    \param  directory   the local directory
    \return             the local filename that contains the header information for the tile that contains the point at <i>latitude</i>, <i>longitude</i>
*/
const string local_header_filename(const double& latitude, const double& longitude, const string& directory)
{ const string lat_string  { to_string(static_cast<int>(latitude + 1.0)) };                                  // assumes two digits
  const string long_string { pad_string(to_string(static_cast<int>(-longitude + 1.0)), 3, PAD_LEFT, '0') };
  
  return ( dirname_with_slash(directory) + "usgs_ned_13_"s + "n"s + lat_string + "w"s + long_string + "_gridfloat.hdr"s) ;
}

/*! \brief              Get the local filename corresponding to the data for particular a tile
    \param  latitude    latitude
    \param  longitude   longitude
    \param  directory   the local directory
    \return             the local filename that contains the data for the tile that contains the point at <i>latitude</i>, <i>longitude</i>
*/
const string local_data_filename(const double& latitude, const double& longitude, const string& directory)
{ const string lat_string  { to_string(static_cast<int>(latitude + 1.0)) };                                  // assumes two digits
  const string long_string { pad_string(to_string(static_cast<int>(-longitude + 1.0)), 3, PAD_LEFT, '0') };
  
  return ( dirname_with_slash(directory) + "usgs_ned_13_"s + "n"s + lat_string + "w"s + long_string + "_gridfloat.flt"s );
}

/*! \brief              Return a base filename derived from latitude and longitude
    \param  latitude    latitude
    \param  longitude   longitude
    \return             the base name of the file that contains the data for the tile that contains the point at <i>latitude</i>, <i>longitude</i>
    
    "nLLwLLL"
*/
const string base_filename(const double& latitude, const double& longitude)
{ const string lat_string  { to_string(static_cast<int>(latitude + 1.0)) };                                  // assumes two digits
  const string long_string { pad_string(to_string(static_cast<int>(-longitude + 1.0)), 3, PAD_LEFT, '0') };

  return ("n"s + lat_string + "w"s + long_string);
}

/*! \brief              Return a base filename derived from latitude and longitude
    \param  llcode      the llcode [lat * 1000 + (+ve)long]
    \return             the base name of the file that contains the data for the tile that contains the point at <i>latitude</i>, <i>longitude</i>
    
    "nLLwLLL"
*/
const std::string base_filename(const int llcode)
{ const string lat_string  { pad_string(to_string(llcode / 1000), 2, PAD_LEFT, '0') };                                  // assumes two digits
  const string long_string { pad_string(to_string(llcode % 1000), 3, PAD_LEFT, '0') };

  return ("n"s + lat_string + "w"s + long_string);
}
