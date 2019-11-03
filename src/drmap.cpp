// $Id: drmap.cpp 15 2019-11-03 15:02:05Z n7dr $

// Released under the GNU Public License, version 2

// Principal author: N7DR

// Copyright owners:
//    N7DR

/* TODO:
      deal with NODATA
      more command line options
        debug
    TOFIX:
*/

/* Internally, calculations are performed in metric units.

   The map projection is an azimuthal equidistant projection centred on the defined QTH.
   
   We apply two transformations to the raw value of elevation data from the USGS at locations other than the QTH:
      1. Allow for change in vertical due to the Earth's curvature => multiply by cosine of the distance angle
      2. Allow for the Earth falling away from the tangent plane at the QTH; this is the curvature_correction function
      
      The end result is that the number in the height field is the "height" measured parallel to the vertical at the QTH.
      As the distance increases, "heights" will therefore naturally decrease, which corresponds to the fact that
      objects further away must be higher to reach the tangent plane at the QTH.
      
      This is different from ordinary maps, but it makes better sense for radio-related calculations.
      
      The correction is small for reasonable distances, and makes no substantive difference to the result.

    The USGS data, which are the 1/3 arc-second elevation data from the National Map,  should have sufficient resolution 
    for most purposes pertaining to HF: data points are ~10m apart. I have seen no estimate from the USGS as to the error 
    associated with their data. My understanding is that the USGS is in the process of augmenting the 1/3 arc-second data
    with national data with a new elevation map with 1-metre resolution, which of course will eliminate any concerns about using
    the current USGS data for HF purposes.  
*/

/*  drmap
      -ant  <antenna height>
      
        The height of the antenna. If -imperial is present, the height is in feet, otherwise it is in metres.
      
      -call <callsign>
      
        The callsign associated with the plot. Must be present.
        
      -cells <number of cells>
      
        The number of cells from the centre of the plot to the edges. The default is 3/8 of the width of the plot, in pixels. For
        the default width of 800, the value is therefore 300.
        
      -datadir <directory>
      
        The directory that contains USGS GridFloat tiles
        
      -elev
      
        Create an elevation plot: the plotted values are the elevation of each cell as seen from the antenna. Most are therefore negative.
        
      -grad
      
        Create a gradient plot: the plotted values are the gradient of the terrain in the direction from the QTH.
        
      -hzn [distance limit]
      
        Plot the elevation of the horizon around the periphery of the figure. Eye-level is set in the same way as eye-level for the
        -los option. Only distances out to the distance limit are used in this calculation. If the distance limit value is not present,
        it is assumed to be the same as the radius of the figure.
        
      -imperial
      
        Use imperial units instead of metric. That is, miles insetad of kilometres and feet instead of metres. Applies both to values
        on the command line and to values on the output plot(s).
        
      -lat <latitude>
      
        Latitude in degrees north. If present, -long should also be present.
        
      -long <longitude>
      
        Longitude in degrees east. If present, -lat should also be present. Note that because the USGS data covers only the
        US, longitude should be negative; but if it is positive, the program will negate the value before use.
        
      -los
      
        Create a line-of-sight plot in addition to the standard height-field plot. Eye-level is assumed to be 1.5m or 5 feet, unless
        the -ant option is presewnt, in which case eye-level is the same as the height of the antenna.

      -outdir <directory>
      
        The directory into which the output maps should be written
        
      -radius <distance1[,distance2[,distance3...]]>
      
        One or more radii for the plot(s), in units of km unless -imperial is present, in which case the units are miles. 
        
      -qthdb <QTH database filename>
      
        A file linking QTH information to callsigns. Each line of the file should contain three entries pertaining to
        a station, separated by white space: the callsign, the latitude and the longitude. This database will be used only
        if one or both of the -lat and -long parameters is missing from the command line.
        
      -sm
      
        USGS tiles are each about 450MB in size. This parameter ("small memory") tells drmap to use the disk files that contain
        the tiles as-is, rather than moving them into RAM where their contents can be accessed much more quickly. Using this parameter
        therefore slows access, but means that there is essentially no limit to the number of tiles that may be used to build a plot. 
        drmap automatically stops loading tiles into RAM when there is less than about 500MB of free RAM and switches to using the tiles
        on disk, so ordinarily there is no need to worry about whether to use the "-sm" parameter. This parameter will be removed in 
        future versions of drmap if it seems to be unneeded in practice.
        
      -width <pixels>
      
        width, in pixels, of the plot(s). The default is 800. The height is automatically set to be three quarters of this value.
        
    Examples:
      drmap -call n7dr -datadir /zfs1/data/usgs/drmap -outdir /tmp/drmap -qthdb ~/radio/qthdb -imperial -ant 50 -radius 2 -los -hzn 5
      
        Look up the call "n7dr" (case is ignored) in the file "~/radio/qthdb". Each line in that file is of the form:
        <callsign>     <latitude>     <longitude>
        
        In particular the fillowing line appears in that file on my system:
        N7DR        40.108016   -105.051700
        
        The latitude and longitude information for N7DR are extracted from that file.
        
        The program will look for relevant USGS files in the directory "/zfs1/data/usgs/drmap". If it fails to find any needed
        files, it will download them from the USGS and place them in that directory prior to using them.
        
        The program will write output plots in the directory tmp/drmap.
        
        The program will use imperial units (miles and feet), and assume an antenna 50 feet above ground.
        
        It will generate a height plot displaying a radius of 2 miles around the N7DR QTH.
        
        It will also create a line-of-sight plot (showing the terrain visible from 50 feet above ground level).
        
        In both plots, the elevation of the horizon as seen from 50 feet above ground level will be drawn around the periphery. The program 
        will assume that no contribution to the horizon is more than five miles from the QTH.
        
      drmap -call RMNP -datadir /zfs1/data/usgs/drmap -outdir /tmp/drmap -lat 40.441358 -long -105.753685 -radius 1
      
        Create a plot for the point 40°.441358N, 105°.753685W (which is in Rocky Mountain National Park).
        
        The program will look for relevant USGS files in the directory "/zfs1/data/usgs/drmap". If it fails to find any needed
        files, it will download them from the USGS and place them in that directory prior to using them.

        The program will write output plots in the directory tmp/drmap.
        
        The program will use metric units (kilometres and metres).
        
        It will generate a height plot displaying a radius of 1 kilometre around the designated location.
        
      drmap -call RMNP -datadir /zfs1/data/usgs/drmap -outdir /tmp/drmap -lat 38.469303 -long -109.739254 -radius 10 -los -hzn 5 -grad
        
        FIX Hzn Eye to say 1.5 instead of 1
        FIX Hzn elevation going up to 65°
        
*/

#include "command_line.h"
#include "diskfile.h"
#include "grid_float.h"
#include "memory.h"
#include "r_figure.h"

#include <complex>
#include <iomanip>
#include <iostream>
#include <set>
#include <thread>

using namespace std;

enum class VISIBILITY { UNKNOWN,
                        VISIBLE,
                        NOT_VISIBLE
                      };

constexpr double MTOF   { 3.28084 };          // metres to feet
constexpr double FTOM   { 1 / MTOF };         // feet to metres
constexpr double KMTOMI { 0.62137119 };       // km to miles
constexpr double MITOKM { 1 / KMTOMI };       // mile to km

const unsigned int N_CPUS { thread::hardware_concurrency() };

bool debug { false };

const string NODATA_COLOUR { "aquamarine4"s };              // colour on plots when data are missing

int    n_cells       { 300 };                                                          // number of cells to be displayed from centre to outside
size_t total_n_cells { static_cast<size_t>( (2 * n_cells + 1) * (2 * n_cells + 1) ) }; // total number of cells on a plot

set<int> tile_llcs;                                             // identifiers for the tiles we will need; we reference tiles by their lat-long codes [lat * 1000 + (+ve)long] 
map<int /* lat-long code */, grid_float_tile> tiles;            // container for the actual tiles we will use

// mutexes
mutex angle_field_mutex;
mutex height_field_mutex;
mutex los_field_mutex;
mutex mean_height_mutex;
mutex tile_llcs_mutex;

// forward declarations
void calculate_needed_tiles(const float& distance_per_square, const pair<double, double>& qth, const bool los, const int delta_y_start, const int delta_y_increment);            ///< determine the needed tiles
void call_lat_long(RInside& R, const string& callsign, const double latitude, const double longitude);
void draw_logo(RInside& R, const double& distance_scale);                                                                                                                        ///< N7DR
void draw_horizon_quadrilaterals(RInside& R, const double& distance_scale, const array<float, 360>& horizon, const value_map<float, int>& vm_horizon, const vector<string>& cv); ///< add horizon quadrilaterals to plot
const float elevation_angle(const double& lat1, const double& long1, const double& lat2, const double& long2, const double& h1, const double& h2);
void label_axes(RInside& R, const vector<int>& distances_km, const vector<int>& distances_in_metres, const string& long_distance_unit_str);
void label_horizon_gradient(RInside& R, const float min_horizon, const float max_horizon, r_colour_gradient& colour_gradient);
void populate_fields(const float& distance_per_square, const pair<double, double>& qth, const int delta_y_start, const int delta_y_increment,
                     vector<vector<float>>& height_field, const float antenna_height, const double& distance_scale, float& sum_terrain_height,
                     int& n_cells_terrain_height, const bool elev, const float raw_qth_height, vector<vector<float>>& angle_field,
                     const bool los, vector<vector<VISIBILITY>>& los_field, const bool grad, vector<vector<float>>& grad_field);

/*  \brief          Calculate the elevation above zero degrees of one point as seen from another
    \param  ll1     latitude and longitude of first point
    \param  ll2     latitude and longitude of second point
    \param  h1      height of first point relative to sphere/geoid
    \param  h2      height of first point relative to sphere/geoid
    \return         the elevation of the second point as seen from the first point

   The USGS "elevation" values are referenced to a geoid. Locally, and for the purpose of this
   calculation, it's sufficient to treat the Earth between the two points as a sphere.
*/
inline const float elevation_angle(const pair<double, double>& ll1, const pair<double, double>& ll2, const double& h1, const double& h2)
  { return elevation_angle(ll1.first, ll1.second, ll2.first, ll2.second, h1, h2); }

// returned in metric
const float command_line_value(const command_line& cl, const string& parameter, const float default_value, const bool imperial)
{ float rv { static_cast<float>(default_value * (imperial ? FTOM : 1)) };

  if (cl.value_present(parameter) and !starts_with(cl.value(parameter), "-"s))  
    rv = from_string<float>(cl.value(parameter)) * (imperial ? FTOM : 1);
    
  return rv;
}

int main(int argc, char** argv)
{ const command_line cl(argc, argv);
 
  if (!cl.value_present("-call"s))
  { cerr << "Error: " << "call not present" << endl;
    exit(-1); 
  }
  
  const string callsign          { to_upper(cl.value("-call")) };
  const string modified_callsign { (contains(callsign, "/") ? replace(callsign, "/", "-") : callsign) };    // can't use a "/" in filenames, so need a modified version
  const string data_directory    { cl.value_present("-datadir"s) ? cl.value("-datadir"s) : "/tmp/drmap"s };
  const string out_directory     { cl.value_present("-outdir"s) ? cl.value("-outdir"s) : "."s };
  
// read values from the command line 
  const bool         imperial { cl.parameter_present("-imperial"s) };  
  const bool         los      { cl.parameter_present("-los"s) };
  const bool         elev     { cl.parameter_present("-elev"s)  or cl.parameter_present("-angle"s)};
  const bool         grad     { cl.parameter_present("-grad"s) };
  
  debug = cl.parameter_present("-v"s) or cl.parameter_present("-debug"s);

  const unsigned int width    { cl.value_present("-width"s) ? from_string<unsigned int>(cl.value("-width"s)) : 800 };
  
  if (width != 800)
    n_cells = (width * 3) / 8;
  
  double latitude        { cl.value_present("-lat"s) ? from_string<double>(cl.value("-lat"s)) : 0 };
  double longitude       { cl.value_present("-long"s) ? -(abs(from_string<double>(cl.value("-long"s)))) : 0 };
  
  const float  antenna_height  { command_line_value(cl, "-ant"s, 0, imperial) };                                                                // metres
  const float  los_height      { command_line_value(cl, "-los"s, (antenna_height ? antenna_height * MTOF : (imperial ? 5 : 1.5)), imperial) };  // metres; 5 => eye_level = 5 feet
  const string qth_db_filename { cl.value_present("-qthdb"s) ? cl.value("-qthdb") : string() };                                                 // currently unused

  const bool  hzn     { cl.parameter_present("-hzn"s) };     // do we draw the horizon?
  const float hzn_eye { hzn ? los_height : 0 };             // height of eye for drawing horizon
  
  double hzn_distance_limit { 0 };                      // cut-off distance for horizon calculation
 
  const string hzn_eye_str { imperial ? to_string(static_cast<int>(round(hzn_eye * MTOF))) : to_string(hzn_eye, 1) };   // string describing height of horizon eye (without unit)

  if (cl.value_present("-cells") and !starts_with(cl.value("-cells"), "-"))
  { n_cells = from_string<int>(cl.value("-cells"));
    total_n_cells = (2 * n_cells + 1) * (2 * n_cells + 1);
  }

  const string distance_unit_str      { (imperial ? "mi"s : "km"s) };
  const string height_unit_str        { (imperial ? "ft"s : "m"s) };
  const string long_distance_unit_str { (imperial ? "miles"s : "km"s) };

  memory_information mem_info;              // so we can see if we are running short of memory when we request to load a tile

// check that something is giving us lat and long
  if ( (!cl.value_present("-lat"s) or !cl.value_present("-long"s)) and !cl.value_present("-qthdb"s))
  { cerr << "No QTH information available; need QTH database or lat/long info" << endl;
    exit(-1);
  }

// try to read lat/long info from QTH file -- only if lat/long not set
  if ( (!cl.value_present("-lat"s) or !cl.value_present("-long"s)) and !qth_db_filename.empty())
  { if (!file_exists(qth_db_filename))
    { cerr << "Error: QTH database file " << qth_db_filename << " does not exist" << endl;
      exit(-1);
    }
    
    const vector<string> lines { squash(to_lines(to_upper(read_file(qth_db_filename))), ' ') };
    
    double db_latitude  { 0 };
    double db_longitude { 0 };
    bool   found_call   { false };
    
    for (unsigned int n = 0; !found_call and n < lines.size(); ++n)
    { const string& line   { lines[n] };
      
      if (!line.empty())
      { const vector<string> fields { split_string(line, ' ') };
    
        if (fields.size() >= 3)
        { if (fields[0] == callsign)
          { db_latitude  = from_string<double>(fields[1]);
            db_longitude = from_string<double>(fields[2]);
            found_call = true;
          }
        }
      }      
    }
    
    if (found_call)
    { latitude = db_latitude;
      longitude = db_longitude;
    }
    else
    { cerr << "Unable to find call " << callsign << " in QTH database file " << qth_db_filename << endl;
      exit(-1);
    }
  }
 
// the default plots -- maybe remove these?
  const vector<double> imperial_distances { 1 * MITOKM * 1000, 2 * MITOKM * 1000, 5 * MITOKM * 1000};   // distances are always in metric units
  const vector<double> metric_distances   { 1000, 2000, 5000, 10000 };
  
  vector<double> cli_distances;         // to hold distances from the command line
  
  if (cl.value_present("-radius"s))
  { const vector<string> radii { split_string(cl.value("-radius"s), ',') };
  
    for (const auto& r : radii)
      cli_distances.push_back(from_string<int>(r) * 1000 * (imperial ? MITOKM : 1));
  }

  vector<double> distances_m { ( cl.value_present("-radius"s) ? cli_distances : (imperial ? imperial_distances : metric_distances) ) };     // radii to plot (in metres)
  
  sort(distances_m.begin(), distances_m.end());         // always go from smallest to largest area
  
  const pair<double, double> qth { latitude, longitude };                                       // the QTH

// debug
  if (debug)
  { for (unsigned int n = 0; n < distances_m.size(); ++n)
      cout << "distances_m[" << n << "] = " << distances_m[n] << endl;
      
    cout << setprecision(12);                                     // so that float/doubles are written with a decent amount of precision
    cout << "QTH = " << latitude << ", " << longitude << endl;
  }

  RInside R { };        // we will need a running instance of R in order to create the plots
 
// the big loop -- generate the height field for a particular distance
  for (const auto& distance_scale : distances_m)
  { const float distance_per_square { static_cast<float>(distance_scale / n_cells) };     // width/height of a cell (in m) along curved surface

// set the farthest limit for the horizon calculation
    if (hzn)
    { if ( !cl.value_present("-hzn"s) or ( (cl.value_present("-hzn"s)) and (starts_with(cl.value("-hzn"s), "-")) ) )      // no explicit value, so use distance
        hzn_distance_limit = distance_scale;
      else
        hzn_distance_limit = from_string<double>(cl.value("-hzn"s)) * 1000 * (imperial ? MITOKM : 1);   // convert to metres
    }

    const string hzn_str { to_string(int( (hzn_distance_limit / (imperial ? (1000 * MITOKM) : 1000) ) + 0.01)) };

// start by figuring out which tiles we need; we do this now in order to allow the main field operations
// to be easily run in multiple threads without having to deal with asynchronous downloads  
    tile_llcs.clear();  
    tile_llcs.insert(llc(qth));                   // we need at least the tile that contains the QTH

    if (debug)
    { cout << "distance per square = " << distance_per_square << endl;
      cout << "calculating needed tiles" << endl;
    }

// in parallel, determine the tiles that are needed
    { vector<future<void>> vec_futures;    

      for (int start = 1; start < static_cast<int>(N_CPUS); ++start)
        vec_futures.emplace_back(async(launch::async, calculate_needed_tiles, distance_per_square, qth, los, (-n_cells + (start - 1)), (N_CPUS - 1)));
    
// hzn is done separately, because it is calculated only once, not per-cell 
      if (hzn)
      { for (int bearing = 0; bearing < 360; bearing += 1)
        { for (int pc = 1; pc <=100; ++pc)
          { const double               distance_to_square_n { (pc * hzn_distance_limit) / 100 };           // assumes hzn_distance_limit isn't something sillily small
            const pair<double, double> ll_n                 { ll_from_bd(qth, bearing, distance_to_square_n) };
            const auto                 lat_long_code        { llc(ll_n) };

            { lock_guard<mutex> tile_llcs_lock(tile_llcs_mutex);
            
              tile_llcs.insert(lat_long_code);
            }
          }
        }
      }
    
      for (auto& this_future : vec_futures)
        this_future.get();                                  // .get() blocks until the future is available
    }

    if (debug)
      cout << "Number of tiles = " << tile_llcs.size() << endl;     // the number of different tiles -- note that we might have missed some, which will be
                                                                    // downloaded later as necessary; decreasing the bearing increment and decreasing the
                                                                    // size of steps along a bearing decreases the probability of missing tiles

    tiles.clear();                                                // remove any memory of what we used on the preceding plot (if any)

// download the new tiles in parallel
    { vector<future<void>> vec_futures;    

      for (const auto& tile_llc : tile_llcs)
        vec_futures.emplace_back(async(launch::async, download_if_necessary, tile_llc, data_directory));
    
      for (auto& this_future : vec_futures)
        this_future.get();                                  // .get() blocks until the future is available
    }
    
// make the tiles available   
    for (const auto& tile_llc : tile_llcs)
      tiles.insert( { tile_llc, move(grid_float_tile(local_header_filename(tile_llc, data_directory), local_data_filename(tile_llc, data_directory), (cl.parameter_present("-sm"s) or (mem_info.mem_available(true) < 500'000'000)))) } );  // I don't know why move doesn't fix the crash
    
    if (debug)
      cout << "Calculating map for distance = " << comma_separated_string(int(distance_scale + 0.5)) << endl;
    
    vector<vector<float>>      angle_field(2 * n_cells + 1, vector<float>(2 * n_cells + 1, 0));                             // the angle-of-elevation field, set to zero
    vector<vector<float>>      grad_field(2 * n_cells + 1, vector<float>(2 * n_cells + 1, 0));                              // the QTH-based gradient field, set to zero
    vector<vector<float>>      height_field(2 * n_cells + 1, vector<float>(2 * n_cells + 1, 0));                            // the actual height field, set to zero; will later INCLUDE antenna in the QTH cell
    vector<vector<VISIBILITY>> los_field(2 * n_cells + 1, vector<VISIBILITY>(2 * n_cells + 1, VISIBILITY::UNKNOWN));        // LOS field, set to UNKNOWN

    float  sum_terrain_height     { 0 };             // used for calculating mean height
    int    n_cells_terrain_height { 0 };             // used for calculating mean height

    const float raw_qth_height { tiles.at(llc(qth)).interpolated_value(qth) };      // so we have it to use to calculate visibility as we step through the cells

    if (debug)
    { cout << "raw QTH height = " << (imperial ? raw_qth_height * MTOF : raw_qth_height) << height_unit_str << endl;          // does not include antenna
    
      if (los)
        cout << "LOS height = " << (imperial ? los_height * MTOF : los_height) << height_unit_str << endl;
    }
    
// step through each cell in the display  
    { vector<future<void>> vec_futures;    

      for (int start = 1; start <= static_cast<int>(N_CPUS); ++start)
        vec_futures.emplace_back(async(launch::async, populate_fields, 
                                distance_per_square, qth, (-n_cells + (start - 1)), (N_CPUS ),
                                ref(height_field), antenna_height, distance_scale, ref(sum_terrain_height),
                                ref(n_cells_terrain_height), elev, raw_qth_height, ref(angle_field),
                                los, ref(los_field), grad, ref(grad_field)));
    
      for (auto& this_future : vec_futures)
        this_future.get();                                  // .get() blocks until the future is available
    }
    
    if (n_cells_terrain_height)         // do we have an average?
    { const float mean_terrain_height       { sum_terrain_height / n_cells_terrain_height };            // does NOT include antenna at QTH
      const float mean_height_above_terrain { raw_qth_height + antenna_height - mean_terrain_height };
    
      if (debug)
        cout << "MHAT = " << (imperial ? mean_height_above_terrain * MTOF : mean_height_above_terrain) << height_unit_str << endl;
    }

// find the extremes of height, for use in calculating the colour gradient; these are in I/O units    
    float min_height { numeric_limits<float>::max() };
    float max_height { numeric_limits<float>::lowest() };
    
    for (const auto& vf : height_field)
    { for (const auto& height : vf)
      { const auto height_wrt_antenna { height - (raw_qth_height + antenna_height) };            // sets zero to the antenna because height_field at QTH INCLUDES antenna

        min_height = min(height_wrt_antenna, min_height);
        max_height = max(height_wrt_antenna, max_height);
      }
    }
    
    if (imperial)
    { min_height *= MTOF;
      max_height *= MTOF;
    }
 
    if (debug)
    { cout << "min height in I/O unit = " << min_height << endl;
      cout << "max height in I/O unit = " << max_height << endl;
    }
   
// round the min/max values
    int   round_min_height { int(min_height) };
    int   round_max_height { int(max_height + 1) };
    float height_diff      { max_height - min_height };
    
    if (debug)
      cout << "height diff = " << height_diff << endl;
    
// fix the scale for the height -- IN I/O UNITS
    if (height_diff < 100)                                      // < 100m/ft difference
    { round_min_height = ( (int(min_height) / 10) - 1) * 10;
      round_max_height = ( (int(max_height) / 10) * 10 ) + 10;
    }
    else                                                        // >= 100m/ft difference
    { const int n_thousands { int(height_diff / 1000) };
      const int factor      { (n_thousands + 1) * 100 };
    
      round_min_height = ( (int(min_height) / factor) - 1) * factor;
      round_max_height = ( (int(max_height) / factor) * factor ) + factor;
    
      if (max_height == 0)        // make sure that rounding errors don't set it to 100
        round_max_height = 0;
    }

    if (signbit(min_height) != signbit(round_min_height))       // make sure that we have the right sign
      round_min_height = -round_min_height;

    if (debug)
    { cout << "round_min_height = " << round_min_height << endl;
      cout << "round_max_height = " << round_max_height << endl;
    }

// horizon
    array<float, 360> horizon;

    if (hzn)
    { for (int bearing = 0; bearing < 360; bearing += 1)
      { horizon[bearing] = numeric_limits<float>::lowest();
    
        for (int pc = 1; pc <=100; ++pc)
        { const double               distance_to_square_n { (pc * hzn_distance_limit) / (100) };
          const pair<double, double> ll_n                 { ll_from_bd(qth, bearing, distance_to_square_n) };
          const float                raw_value_n          { tiles.at(llc(ll_n)).interpolated_value(ll_n) };
          const float                angle_n              { elevation_angle(qth, ll_n, raw_qth_height + antenna_height, raw_value_n) };              
          
          horizon[bearing] = max(horizon[bearing], angle_n);        
        }
      
        horizon[bearing] *= RTOD;     // convert to degrees
      }
    }
    
    const float min_horizon { floor(MIN_ELEMENT(horizon)) };
    const float max_horizon { floor(MAX_ELEMENT(horizon) + 1) };
    
    if (debug)
    { cout << "min horizon = " << min_horizon << endl;
      cout << "max horizon = " << max_horizon << endl;
    }

    const value_map<float, int> vm_horizon(min_horizon, max_horizon, 0 /* min index into cv */, 999 /* max index into cv */);
    
 // image(s)
    const vector<array<float, 4>> screen_definitions { { 0.0, 0.75, 0.0, 1.0 },
                                                       { 0.71, 0.82, 0.0, 1.0 },        // overlap!
                                                       { 0.82, 1.0, 0.0, 1.0 }
                                                     };

    const string distance_str { to_string(static_cast<int>( (distance_scale + 1) * (imperial? (MTOF / 5280) : (1.0 / 1000) ) ) ) };

// the basic height map
    
    create_figure(R, out_directory + "/drmap-"s + modified_callsign + "-" + distance_str + distance_unit_str + ".png"s, width, ( (3 * width) / 4 ));
    create_screens(R, screen_definitions);
    select_screen(R, 1);
    
    execute_r( R, "par(mar = c(2.5, 2.5, 2.5, 2.5))" ); // default = c(5.1, 4.1, 4.1, 2.1) ... square plot
    
    start_plot<int, int>(R, -distance_scale, distance_scale, -distance_scale, distance_scale);
    
    const double rect_width  { distance_scale / (n_cells) };
    const double rect_height { distance_scale / (n_cells) };
   
    set_rect(R, "black"s);

    r_colour_gradient colour_gradient(R, { "grey"s, "brown"s, "green"s, "yellow"s, "red"s, "blue"s, "white"s });
      
    const vector<string>        cv { colour_gradient.colour_vector() };
    
    const value_map<float, int> vm(round_min_height, round_max_height, 0 /* min index into cv */, 999 /* max index into cv */);
    
    { r_rects<float> cells(R, total_n_cells);
      
      for (int n_row = 0; n_row < static_cast<int>(height_field.size()); ++n_row)            // rows go from S to N
      { const auto& row { height_field[n_row] };
    
        for (int n_column = 0; n_column < static_cast<int>(row.size()); ++n_column)          // columns go from W to E
        { const float height_wrt_antenna { row[n_column] - (raw_qth_height + antenna_height) };
      
          string clr;
          
          if (height_wrt_antenna < -9000)
            clr = NODATA_COLOUR;
          else
            clr = (cv[vm.map_value( imperial ? height_wrt_antenna * MTOF : height_wrt_antenna ) ]);
      
          cells.add(-distance_scale + (n_column - 0.5) * rect_width, -distance_scale + (n_column + 0.5) * rect_width, 
                    -distance_scale + (n_row - 0.5) * rect_height, -distance_scale + (n_row + 0.5) * rect_height,
                    clr);
        }
      }
      
      cells.draw();
    }

    if (hzn)
      draw_horizon_quadrilaterals(R, distance_scale, horizon, vm_horizon, cv);

    draw_logo(R, distance_scale);
   
 // create distance axes   
    const double ds { round( imperial ? distance_scale * KMTOMI : distance_scale ) };
    
    vector<int> distances_km { (ds < 10000) ? r_seq( static_cast<int>(-ds / 1000), static_cast<int>(ds / 1000) ) :
                                  ( (ds < 20000) ? r_seq( static_cast<int>(-ds / 1000), static_cast<int>(ds / 1000), 2 ) :
                                  ( (ds < 50000) ? r_seq( static_cast<int>(-ds / 1000), static_cast<int>(ds / 1000), 5 ) :
                                  ( (ds < 100000) ? r_seq( static_cast<int>(-ds / 1000), static_cast<int>(ds / 1000), 10 ) :
                                                    r_seq( static_cast<int>(-ds / 1000), static_cast<int>(ds / 1000), 20 ) ) ) ) };
                                                          
 // calculate the actual locations of the ticks and labels; this is (always) measured in metres
    const int rds { static_cast<int>(distance_scale + 0.01) };

    vector<int> distances_in_metres { (ds < 10000) ? r_seq( -rds, rds, static_cast<int>(1000.01 * (imperial ? MITOKM : 1)) ) :
                                      (ds < 20000) ? r_seq( -rds, rds, static_cast<int>(2000.01 * (imperial ? MITOKM : 1)) ) :
                                      (ds < 50000) ? r_seq( -rds, rds, static_cast<int>(5000.01 * (imperial ? MITOKM : 1)) ) :
                                      (ds < 100000) ? r_seq( -rds, rds, static_cast<int>(10000.01 * (imperial ? MITOKM : 1)) ) :
                                                      r_seq( -rds, rds, static_cast<int>(20000.01 * (imperial ? MITOKM : 1)) ) };

    label_axes(R, distances_km, distances_in_metres, long_distance_unit_str);

    execute_r(R, "require(plotrix, quietly = TRUE)");
    execute_r(R, "draw.circle(0, 0, " + to_string(distance_scale / 100) + ", col = 'ORANGE')");     // mark the QTH
    execute_r(R, "draw.circle(0, 0, " + to_string(distance_scale) + ", border = 'BLACK')");         // mark the radius

// gradient
    colour_gradient.display_all_on_second_screen("Rel\nHt(" + height_unit_str + ")", colour_gradient.labels(round_min_height, round_max_height));

    if (hzn)
      label_horizon_gradient(R, min_horizon, max_horizon, colour_gradient);

    select_screen(R, 3);
    r_function(R, "par", "mar = rep(0, 4)"s);
    start_plot<int, int>(R, 0, 1);
    
    call_lat_long(R, callsign, latitude, longitude);

    if (antenna_height != 0)
      execute_r(R, "text(x=0.50, y = 0.05, labels = c('Ant = " + cl.value("-ant"s) + height_unit_str + "'), cex = 1.2)");

    if (n_cells_terrain_height)
    { const float  mean_terrain_height       { sum_terrain_height / n_cells_terrain_height };
      const float  mean_height_above_terrain { static_cast<float>((raw_qth_height + antenna_height - mean_terrain_height) * (imperial ? MTOF : 1)) };   
      const string displayable_height        { (imperial ? to_string(static_cast<int>(mean_height_above_terrain + 0.5)) : to_string(int( (mean_height_above_terrain * 10) + 0.5) / 10)) };
      const string scale                     { to_string(int( (distance_scale / (imperial ? (1000 * MITOKM) : 1000) ) + 0.01)) };
   
      execute_r(R, "text(x=0.50, y = 0.10, labels = c('MHAT(" + scale + distance_unit_str + ") = " + displayable_height + height_unit_str + "'), cex = 1.2)");
    }
    
    if (hzn)
    { execute_r(R, "text(x=0.50, y = 0.15, labels = c('Hzn Lmt = " + hzn_str + distance_unit_str + "'), cex = 1.2)");
      execute_r(R, "text(x=0.50, y = 0.20, labels = c('Hzn Eye = " + hzn_eye_str + height_unit_str + "'), cex = 1.2)");
    }

    execute_r(R, "graphics.off()"s);
    
    if (los)
    { if (debug)
        cout << "LOS plot" << endl;
 
      create_figure(R, out_directory + "/drmap-"s + modified_callsign + "-" + distance_str + distance_unit_str + "-los.png"s, width, ( (3 * width) / 4 ));
      create_screens(R, screen_definitions);
      select_screen(R, 1);
    
      execute_r( R, "par(mar = c(2.5, 2.5, 2.5, 2.5))" ); // default = c(5.1, 4.1, 4.1, 2.1) ... square plot
    
      start_plot<int, int>(R, -distance_scale, distance_scale, -distance_scale, distance_scale);
      set_rect(R, "black"s);

      { r_rects<float> cells(R, total_n_cells);
      
        for (int n_row = 0; n_row < static_cast<int>(height_field.size()); ++n_row)            // rows go from S to N
        { const auto& row { height_field[n_row] };
    
          for (int n_column = 0; n_column < static_cast<int>(row.size()); ++n_column)          // columns go from W to E
          { const float       height_wrt_antenna { row[n_column] - (raw_qth_height + antenna_height) };
            const VISIBILITY& visibility         { los_field[n_row][n_column] };
      
            cells.add(-distance_scale + (n_column - 0.5) * rect_width, -distance_scale + (n_column + 0.5) * rect_width, 
                      -distance_scale + (n_row - 0.5) * rect_height, -distance_scale + (n_row + 0.5) * rect_height,
                      ( (visibility == VISIBILITY::VISIBLE) ? cv[vm.map_value( imperial ? height_wrt_antenna * MTOF : height_wrt_antenna ) ] : "black"));
          }
        }
      
        cells.draw();
      }
   
      if (hzn) 
        draw_horizon_quadrilaterals(R, distance_scale, horizon, vm_horizon, cv);

      draw_logo(R, distance_scale);
      label_axes(R, distances_km, distances_in_metres, long_distance_unit_str);

      execute_r(R, "require(plotrix, quietly = TRUE)");                                           // just to make it easier to draw circles
      execute_r(R, "draw.circle(0, 0, " + to_string(distance_scale / 100) + ", col = 'ORANGE')"); // QTH marker
      execute_r(R, "draw.circle(0, 0, " + to_string(distance_scale) + ", border = 'BLACK')");     // radius marker

// gradient
      colour_gradient.display_all_on_second_screen("Rel\nHt(" + height_unit_str + ")", colour_gradient.labels(round_min_height, round_max_height));

      if (hzn)
        label_horizon_gradient(R, min_horizon, max_horizon, colour_gradient);

      select_screen(R, 3);    
      r_function(R, "par", "mar = rep(0, 4)"s);
      start_plot<int, int>(R, 0, 1);
      
      call_lat_long(R, callsign, latitude, longitude);

      if (antenna_height != 0)
        execute_r(R, "text(x=0.50, y = 0.05, labels = c('Ant = " + cl.value("-ant"s) + height_unit_str + "'), cex = 1.2)");

      if (n_cells_terrain_height)
      { const float  mean_terrain_height       { sum_terrain_height / n_cells_terrain_height };
        const float  mean_height_above_terrain { static_cast<float>((raw_qth_height + antenna_height - mean_terrain_height) * (imperial ? MTOF : 1)) };   
        const string displayable_height        { (imperial ? to_string(static_cast<int>(mean_height_above_terrain + 0.5)) : to_string(int( (mean_height_above_terrain * 10) + 0.5) / 10)) };
        const string scale                     { to_string(int( (distance_scale / (imperial ? (1000 * MITOKM) : 1000) ) + 0.01)) };
        
        const string los_eye_str { imperial ? to_string(static_cast<int>(round(los_height * MTOF))) : to_string(los_height, 1) };   // string describing height of LOS eye (without unit)

        execute_r(R, "text(x=0.50, y = 0.10, labels = c('MHAT(" + scale + distance_unit_str + ") = " + displayable_height + height_unit_str + "'), cex = 1.2)");
        execute_r(R, "text(x=0.50, y = 0.15, labels = c('LOS Eye = " + los_eye_str + height_unit_str + "'), cex = 1.2)");

        if (hzn)
        { execute_r(R, "text(x=0.50, y = 0.20, labels = c('Hzn Lmt = " +  hzn_str + distance_unit_str + "'), cex = 1.2)");
          execute_r(R, "text(x=0.50, y = 0.25, labels = c('Hzn Eye = " + hzn_eye_str + height_unit_str + "'), cex = 1.2)");
        }
      }

      execute_r(R, "graphics.off()"s);
    }
    
    if (elev)
    { if (debug)
        cout << "Angle plot" << endl;
        
      const value_map<float, int> vm_angle(-5, 5, 0 /* min index into cv */, 999 /* max index into cv */);        // 10 just for now

      create_figure(R, out_directory + "/drmap-"s + modified_callsign + "-" + distance_str + distance_unit_str + "-elev.png"s, width, ( (3 * width) / 4 ));
      create_screens(R, screen_definitions);
      select_screen(R, 1);
    
      execute_r( R, "par(mar = c(2.5, 2.5, 2.5, 2.5))" ); // default = c(5.1, 4.1, 4.1, 2.1) ... square plot
    
      start_plot<int, int>(R, -distance_scale, distance_scale, -distance_scale, distance_scale);
      set_rect(R, "black"s);
      
// use ranked angles instead of absolute values in order to linearise the gradient
      vector<float> angles;
      
      angles.reserve(total_n_cells);
      
      for (int n_row = 0; n_row < static_cast<int>(angle_field.size()); ++n_row)            // rows go from S to N
      { const auto& row { angle_field[n_row] };
    
        for (int n_column = 0; n_column < static_cast<int>(row.size()); ++n_column)          // columns go from W to E
          angles.push_back(row[n_column]);
      }

      sort(angles.begin(), angles.end());
      
      auto mapped_angle = [=](const float& a) 
        { const auto it { lower_bound(angles.cbegin(), angles.cend(), a) };
          const auto d  { std::distance(angles.cbegin(), it) };
          
          return static_cast<int>( ( (d * 1.0) / (angles.size() - 1) ) * 999  );        // element number in the gradient 
        };
  
      { r_rects<float> cells(R, total_n_cells);
      
        for (int n_row = 0; n_row < static_cast<int>(angle_field.size()); ++n_row)            // rows go from S to N
        { const auto& row { angle_field[n_row] };
    
          for (int n_column = 0; n_column < static_cast<int>(row.size()); ++n_column)          // columns go from W to E
          { const float& angle = row[n_column];
            
            cells.add(-distance_scale + (n_column - 0.5) * rect_width, -distance_scale + (n_column + 0.5) * rect_width, 
                      -distance_scale + (n_row - 0.5) * rect_height, -distance_scale + (n_row + 0.5) * rect_height,
                      cv.at(mapped_angle(angle)) );
          }
        }
      
        cells.draw();
      }
      
      if (hzn)
        draw_horizon_quadrilaterals(R, distance_scale, horizon, vm_horizon, cv);

      draw_logo(R, distance_scale);
      label_axes(R, distances_km, distances_in_metres, long_distance_unit_str);

      execute_r(R, "require(plotrix, quietly = TRUE)");                                           // just to make it easier to draw circles
      execute_r(R, "draw.circle(0, 0, " + to_string(distance_scale / 100) + ", col = 'ORANGE')"); // QTH marker
      execute_r(R, "draw.circle(0, 0, " + to_string(distance_scale) + ", border = 'BLACK')");     // radius marker

// we can't use the usual canned routines to display the gradient because there's no simple value-based mapping
      select_screen(R, 2);
      r_function(R, "par", "mar = rep(0, 4)"s);
      start_plot<int, int>(R, 0, 2);

      constexpr float x { 0.8 }; 
      constexpr float y { 0.95 };
  
      execute_r(R, "text(x = "s + to_string(x) + ", y = "s + to_string(y) + ", labels = '"s + "Elev\nAngle(°)" + "', pos = 4)"s);

      colour_gradient.display();
      
      constexpr int n_labels { 21 };          // need lots of labels because of the nonlinearity
      
      vector<string> angle_labels_str;
      
      for (size_t n_label = 0; n_label < n_labels; ++n_label)
      { const size_t labels_index { static_cast<size_t>( ((n_label * 1.0) / (n_labels - 1)) * (angles.size() - 1)) };
        
        stringstream stream;
        
        stream << fixed << setprecision(2) << angles.at(labels_index);
        
        angle_labels_str.push_back( stream.str() );
      }
      
      R["y_labels"s] = angle_labels_str;
      
      const float delta_y { (colour_gradient.gradient_top() - colour_gradient.gradient_bottom()) / (n_labels - 1) };
      
      execute_r(R, "y_labels_at <- seq("s + to_string(colour_gradient.gradient_bottom()) +
                   ", by = "s + to_string(delta_y) + ", length.out = "s + to_string(n_labels) + ")"s);

      execute_r(R, "text(x=0.80, y = y_labels_at, labels = y_labels, pos = 4)"s);
     
      if (hzn)
        label_horizon_gradient(R, min_horizon, max_horizon, colour_gradient);

      select_screen(R, 3);
      r_function(R, "par", "mar = rep(0, 4)"s);
      start_plot<int, int>(R, 0, 1);

      call_lat_long(R, callsign, latitude, longitude);

      if (antenna_height != 0)
        execute_r(R, "text(x=0.50, y = 0.05, labels = c('Ant = " + cl.value("-ant"s) + height_unit_str + "'), cex = 1.2)");

      if (n_cells_terrain_height)
      { const float mean_terrain_height       { sum_terrain_height / n_cells_terrain_height };
        const float mean_height_above_terrain { static_cast<float>((raw_qth_height - mean_terrain_height) * (imperial ? MTOF : 1)) };   
        const string displayable_height       { (imperial ? to_string(static_cast<int>(mean_height_above_terrain + 0.5)) : to_string(int( (mean_height_above_terrain * 10) + 0.5) / 10)) };
        const string scale                    { to_string(int( (distance_scale / (imperial ? (1000 * MITOKM) : 1000) ) + 0.01)) };
   
        execute_r(R, "text(x=0.50, y = 0.10, labels = c('MHAT(" + scale + distance_unit_str + ") = " + displayable_height + height_unit_str + "'), cex = 1.2)");
      }
    
      if (hzn)
      { execute_r(R, "text(x=0.50, y = 0.15, labels = c('Hzn Lmt = " +  hzn_str + distance_unit_str + "'), cex = 1.2)");
        execute_r(R, "text(x=0.50, y = 0.20, labels = c('Hzn Eye = " + hzn_eye_str + height_unit_str + "'), cex = 1.2)");
      }
       
      execute_r(R, "graphics.off()"s);
    }
    
    if (grad)
    { if (debug)
        cout << "Gradient plot" << endl;
        
      float min_gradient { numeric_limits<float>::max() };
      float max_gradient { numeric_limits<float>::lowest() };

      for (int delta_y = -n_cells; delta_y <= n_cells; ++delta_y)
      { for (int delta_x = -n_cells; delta_x <= n_cells; ++delta_x)
        { const int                  column_index              { delta_x + n_cells };
          const int                  row_index                 { delta_y + n_cells };
      
          min_gradient = min(min_gradient, grad_field[row_index][column_index]);
          max_gradient = max(max_gradient, grad_field[row_index][column_index]);
        }
      }
  
      if (debug)
      { cout << "min gradient = " << min_gradient << endl;
        cout << "max gradient = " << max_gradient << endl;
      }
      
      min_gradient = floor(min_gradient * 10) / 10;
      max_gradient = floor((max_gradient + 0.1) * 10) / 10;

      if (debug)
      { cout << "plot min gradient = " << min_gradient << endl;
        cout << "plot max gradient = " << max_gradient << endl;
      }

      const value_map<float, int> vm_gradient(min_gradient, max_gradient, 0 /* min index into cv */, 999 /* max index into cv */);

      create_figure(R, out_directory + "/drmap-"s + modified_callsign + "-" + distance_str + distance_unit_str + "-grad.png"s, width, ( (3 * width) / 4 ));
      create_screens(R, screen_definitions);
      select_screen(R, 1);
    
      execute_r( R, "par(mar = c(2.5, 2.5, 2.5, 2.5))" ); // default = c(5.1, 4.1, 4.1, 2.1) ... square plot
    
      start_plot<int, int>(R, -distance_scale, distance_scale, -distance_scale, distance_scale);
      set_rect(R, "black"s);
  
      { r_rects<float> cells(R, total_n_cells);
      
        for (int n_row = 0; n_row < static_cast<int>(grad_field.size()); ++n_row)            // rows go from S to N
        { const auto& row { grad_field[n_row] };
    
          for (int n_column = 0; n_column < static_cast<int>(row.size()); ++n_column)          // columns go from W to E
          { const float& grad = row[n_column];
            
            cells.add(-distance_scale + (n_column - 0.5) * rect_width, -distance_scale + (n_column + 0.5) * rect_width, 
                      -distance_scale + (n_row - 0.5) * rect_height, -distance_scale + (n_row + 0.5) * rect_height,
                      cv.at(vm_gradient(grad)) );
          }
        }
      
        cells.draw();
      }
      
      if (hzn)
        draw_horizon_quadrilaterals(R, distance_scale, horizon, vm_horizon, cv);

      draw_logo(R, distance_scale);
      label_axes(R, distances_km, distances_in_metres, long_distance_unit_str);

      execute_r(R, "require(plotrix, quietly = TRUE)");                                           // just to make it easier to draw circles
      execute_r(R, "draw.circle(0, 0, " + to_string(distance_scale / 100) + ", col = 'ORANGE')"); // QTH marker
      execute_r(R, "draw.circle(0, 0, " + to_string(distance_scale) + ", border = 'BLACK')");     // radius marker
      
      colour_gradient.display_all_on_second_screen("Grad", colour_gradient.labels(min_gradient, max_gradient));

      select_screen(R, 3);
      r_function(R, "par", "mar = rep(0, 4)"s);
      start_plot<int, int>(R, 0, 1);

      call_lat_long(R, callsign, latitude, longitude);

      if (antenna_height != 0)
        execute_r(R, "text(x=0.50, y = 0.05, labels = c('Ant = " + cl.value("-ant"s) + height_unit_str + "'), cex = 1.2)");

      if (n_cells_terrain_height)
      { const float mean_terrain_height       { sum_terrain_height / n_cells_terrain_height };
        const float mean_height_above_terrain { static_cast<float>((raw_qth_height - mean_terrain_height) * (imperial ? MTOF : 1)) };   
        const string displayable_height       { (imperial ? to_string(static_cast<int>(mean_height_above_terrain + 0.5)) : to_string(int( (mean_height_above_terrain * 10) + 0.5) / 10)) };
        const string scale                    { to_string(int( (distance_scale / (imperial ? (1000 * MITOKM) : 1000) ) + 0.01)) };
   
        execute_r(R, "text(x=0.50, y = 0.10, labels = c('MHAT(" + scale + distance_unit_str + ") = " + displayable_height + height_unit_str + "'), cex = 1.2)");
      }
    
      if (hzn)
      { execute_r(R, "text(x=0.50, y = 0.15, labels = c('Hzn Lmt = " +  hzn_str + distance_unit_str + "'), cex = 1.2)");
        execute_r(R, "text(x=0.50, y = 0.20, labels = c('Hzn Eye = " + hzn_eye_str + height_unit_str + "'), cex = 1.2)");
      }
      
      execute_r(R, "graphics.off()"s);
    }
  }
  
  return 0;
}

/*! \brief                          Determine, in parallel, the needed tiles
    \param  distance_per_square     size of a cell, in metres
    \param  qth                     latitude and longitude of the QTH
    \param  los                     whether to perform line-of-sight calculation
    \param  delta_y_start           the starting y offset (the plot starts at -cells)
    \param  delta_y_increment       the number of rows by which to increment y
    
    Calculations relating to hzn are not performed, as those need to be done only once, not per-cell
*/
void calculate_needed_tiles(const float& distance_per_square, const pair<double, double>& qth, const bool los, const int delta_y_start, const int delta_y_increment)
{ for (int delta_y = delta_y_start; delta_y <= n_cells; delta_y += delta_y_increment)
  { for (int delta_x = -n_cells; delta_x <= n_cells; ++delta_x)
    { const double               bearing_from_north        { bearing(delta_x, delta_y) };
      const double               distance_to_square        { sqrt(1.0 * delta_x * delta_x + 1.0 * delta_y * delta_y) * distance_per_square };    // along curved surface
      const pair<double, double> ll                        { ll_from_bd(qth, bearing_from_north, distance_to_square) };
      const auto                 lat_long_code             { llc(ll) };
      
      { lock_guard<mutex> tile_llcs_lock(tile_llcs_mutex);
      
        tile_llcs.insert(lat_long_code);
      }
        
      if (los)
      { if (delta_x != 0 or delta_y != 0)                     // for everything except the QTH cell
        { int decrement { 1 };
            
// a bit of a fudge for very close-in terminating points
          if (distance_to_square < 250)               // 250m
            decrement = max(int(distance_to_square / 4), 1);  // ~25% per step
            
          for (int n = 95; n >= 5; n -= decrement)                                            // skip points near ends to avoid rounding problems
          { const double               distance_to_square_n        { (n * distance_to_square) / (100) };
            const pair<double, double> ll_n                        { ll_from_bd(qth, bearing_from_north, distance_to_square_n) };
            const auto                 lat_long_code               { llc(ll_n) };

            { lock_guard<mutex> tile_llcs_lock(tile_llcs_mutex);
            
              tile_llcs.insert(lat_long_code);
            }
          }
        }
      }
    }
  }
  
  return;
}

/*! \brief                  Draw the horizon quadrilaterals around the periphery of the plot
    \param  R               the R instance
    \param  distance_scale  the radius of the plot, in metres
    \param  horizon         the elevation angles, one per degree
    \param  vm_horizon      the angle-to-colour-index mapping
    \param  cv              the index-to-colour-string mapping
*/
void draw_horizon_quadrilaterals(RInside& R, const double& distance_scale, const array<float, 360>& horizon, const value_map<float, int>& vm_horizon, const vector<string>& cv)
{ const auto delta_1 { distance_scale * 0.02 };             // used for location of inner line [4 -- 1]
  const auto delta_2 { distance_scale * 0.05 };             // used for location of outer line [2 -- 3]
      
  const auto opdm_1  { distance_scale + delta_1 };          // location of inner line, in plot coordinates
  const auto opdm_2  { distance_scale + delta_2 };          // location of outer line, in plot coordinates
          
  constexpr size_t N_BEARINGS { 360 };
          
  vector<float> x, y;
  vector<string> clr;
  
  x.reserve(N_BEARINGS * 5);
  y.reserve(N_BEARINGS * 5);
  clr.reserve(N_BEARINGS);
  
/// push back the x or y coordinates of the points [1, 2, 3, 4] into v
  auto pb = [](vector<float>& v, const float p1, const float p2, const float p3, const float p4)
    { v.push_back( p1 );
      v.push_back( p2 );
      v.push_back( p3 );
      v.push_back( p4 );
      v.push_back(NA_REAL);
    };
    
  for (int bearing = 0; bearing < 360; bearing += 1)
  { clr.push_back(cv[vm_horizon.map_value(horizon[bearing])]);      // the colour of this quadrilateral

    const auto theta_1 { bearing * DTOR };                      // most anticlockwise angle [1, 2]
    const auto theta_2 { (bearing + 1) * DTOR };                // most clockwise angle [3, 4]

    const auto t1 { tan(theta_1) };
    const auto t2 { tan(theta_2) };

    if ( (bearing < 45) or (bearing >= 315) )           // top quadrant
    { 
// x and y coordinates of [1, 2, 3, 4]      
      pb(x, opdm_1 * t1, opdm_2 * t1, opdm_2 * t2, opdm_1 * t2 );
      pb(y, opdm_1,      opdm_2,      opdm_2,      opdm_1 );
    }  
    
    if ( (bearing >= 45) and (bearing < 135) )      // right quadrant
    { pb(x, opdm_1,      opdm_2,      opdm_2,      opdm_1 );
      pb(y, opdm_1 / t1, opdm_2 / t1, opdm_2 / t2, opdm_1 / t2 );
    }
  
    if ( (bearing >= 135) and (bearing < 225) )     // bottom quadrant
    { pb(x, -opdm_1 * t1, -opdm_2 * t1, -opdm_2 * t2, -opdm_1 * t2 );
      pb(y, -opdm_1,      -opdm_2,      -opdm_2,      -opdm_1 );
    }     
 
    if ( (bearing >= 225) and (bearing < 315) )     // left quadrant
    { pb(x, -opdm_1,      -opdm_2,      -opdm_2,      -opdm_1 );
      pb(y, -opdm_1 / t1, -opdm_2 / t1, -opdm_2 / t2, -opdm_1 / t2 );
    }  
  }

// plot all the quadrilaterals      
  R["xpol"] = x;
  R["ypol"] = y;
  R["clrpol"] = clr;
        
  execute_r( R, "polygon( x = xpol, y = ypol, col = clrpol )" );
      
// cleanup R
  execute_r ( R, "xpol <-NULL" );
  execute_r ( R, "ypol <-NULL" );
  execute_r ( R, "clrpol <-NULL" );
}

/*! \brief                  Draw the logo -- PLEASE DO NOT REMOVE THIS, OR CALLS TO IT
    \param  R               the R instance
    \param  distance_scale  the radius of the plot, in metres
*/
void draw_logo(RInside& R, const double& distance_scale)
{ constexpr float l { 0.80f };
  constexpr float r { 1.00f };
  constexpr float b { -1.00f };
  constexpr float t { -0.90f };
    
  r_rect rect( R, distance_scale * l,
                  distance_scale * r,
                  distance_scale * b,
                  distance_scale * t,
                  "WHITE"s,
                  "dark green"s
             );

  rect.draw();
        
  const float x { static_cast<float>(0.90 * distance_scale) };
  const float y { static_cast<float>(-0.95 * distance_scale) };
    
  execute_r(R, "text(x = "s + to_string(x) + ", y = "s + to_string(y) + ", labels = '"s + "N7DR" + "', col = 'dark green', cex = 1.2, font = 2)"s);  // bold
}

/*  \brief          Calculate the elevation above zero degrees of one point as seen from another
    \param  lat1    latitude of first point
    \param  long1   longitude of first point
    \param  lat2    latitude of second point
    \param  long2   longitude of second point
    \param  h1      height of first point relative to sphere/geoid
    \param  h2      height of first point relative to sphere/geoid
    \return         the elevation of the second point as seen from the first point

   The USGS "elevation" values are referenced to a geoid. Locally, and for the purpose of this
   calculation, it's sufficient to treat the Earth between the two points as a sphere.
   
   Assuming a negative elevation angle (i.e. OD = OB + BD, both +ve)
   O = centre of Earth
   RE = radius of Earth
   A = top of antenna (RE + h1)
   OD = RE + h2 [== OB] projected on to horizontal plane through A
   B = top of point 2 (i.e., r + h2)
   theta = distance along surface between the points / radius of Earth
*/
const float elevation_angle(const double& lat1, const double& long1, const double& lat2, const double& long2, const double& h1, const double& h2)
{ const double d     { distance(lat1, long1, lat2, long2) };
  const double theta { d / RE };   // radians; annoyoingly, g++ doesn't properly support Unicode in the names of variables
  const double OD    { (RE + h1) / cos(theta) };
  const double AD    { (RE + h1) * tan(theta) };
  const double BD    { OD - (RE + h2) };
  const double AB    { sqrt(AD * AD + BD * BD - 2 * AD * BD * sin(theta)) };     // cosine rule
  const double alpha { -asin( (BD * cos(theta)) / AB ) };                     // sine rule; the - sign corrects for above/below horizontal
  
  return static_cast<float>(alpha);
}

/*! \brief                          Populate all the fields
    \param  distance_per_square     size of a cell, in metres
    \param  qth                     latitude and longitude of the QTH
    \param  delta_y_start           the starting y offset (the plot starts at -cells)
    \param  delta_y_increment       the number of rows by which to increment y
    \param  height_field            the height field
    \param  antenna_height          the height of the antenna (in metres)
    \param  distance_scale          the radius of the plot, in metres
    \param  sum_terrain_height      the sum of the terrain height for the cells 
    \param  n_cells_terrain_height  the number of cells whose height is included in <i>sum_terrain_height</i>
    \param  elev                    whether to create an elevation plot
    \param  raw_qth_height          the USGS value for the QTH
    \param  angle_field             the elev/angle field
    \param  los                     whether to create a line-of-sight plot
    \param  los_field               the line-of-sight field
    \param  grad                    whether to create a gradient plot
    \param  grad_field              the gradient field
    
    This function is thread-safe. It does not yet handle the NODATA case reasonably.
*/
void populate_fields(const float& distance_per_square, const pair<double, double>& qth, const int delta_y_start, const int delta_y_increment,
                     vector<vector<float>>& height_field, const float antenna_height, const double& distance_scale, float& sum_terrain_height,
                     int& n_cells_terrain_height, const bool elev, const float raw_qth_height, vector<vector<float>>& angle_field,
                     const bool los, vector<vector<VISIBILITY>>& los_field, const bool grad, vector<vector<float>>& grad_field)
{ for (int delta_y = delta_y_start; delta_y <= n_cells; delta_y += delta_y_increment)
  { for (int delta_x = -n_cells; delta_x <= n_cells; ++delta_x)
    { const int                  column_index              { delta_x + n_cells };
      const int                  row_index                 { delta_y + n_cells };
      const double               bearing_from_north        { bearing(delta_x, delta_y) };
      const double               distance_to_square        { sqrt(1.0 * delta_x * delta_x + 1.0 * delta_y * delta_y) * distance_per_square };    // along curved surface
      const pair<double, double> ll                        { ll_from_bd(qth, bearing_from_north, distance_to_square) };        
      const double               correction                { curvature_correction(distance_to_square) };

      float raw_value { -9999 };        // default value is NODATA
      
      try
      { raw_value = tiles.at(llc(ll)).interpolated_value(ll);                 // height per USGS

// see note near the top of the file regarding modification of the received heights
        { lock_guard<mutex> height_field_lock(height_field_mutex);                    // should not be necessary, but be paranoid
      
          height_field[row_index][column_index] = raw_value * cos(distance_to_square / RE) - correction;
        
          if ( (delta_x == 0) and (delta_y == 0) )
            height_field[row_index][column_index] += antenna_height;              // add the antenna to the central square
        }
        
        if (distance_to_square <= distance_scale)                           // accumulate for calculation of MHAT
        { lock_guard<mutex> mean_height_lock(mean_height_mutex);
      
          sum_terrain_height += height_field[row_index][column_index];      // adds antenna height to QTH square
        
          if ( (delta_x == 0) and (delta_y == 0) )
            sum_terrain_height -= antenna_height;                           // remove the antenna from the central square, so it's RAW terrain

          n_cells_terrain_height++;
        }
      }
      
      catch (const grid_float_error& e)
      { cerr << "Caught grid float error while calculating height field: " << e.reason() << endl;
          
        lock_guard<mutex> height_field_lock(height_field_mutex);                    // should not be necessary, but be paranoid
      
        height_field[row_index][column_index] = -9999;
      }
        
      double elevation_angle_in_degrees { 0 };
      
      if (elev)
      { if (raw_value > -9000)
        { elevation_angle_in_degrees = elevation_angle(qth, ll, raw_qth_height + antenna_height, raw_value) * RTOD;
        
          { lock_guard<mutex> angle_field_lock(angle_field_mutex);                    // should not be necessary, but be paranoid
        
            angle_field[row_index][column_index] = elevation_angle_in_degrees;
          }
        }
        else    // NODATA
        { lock_guard<mutex> angle_field_lock(angle_field_mutex);                    // should not be necessary, but be paranoid
        
          angle_field[row_index][column_index] = -9999;
        }
      }
 
      if (grad)
      { if ( (delta_x == 0) and (delta_y == 0) )
          grad_field[row_index][column_index] = 0;
        else
        { try
          { const float delta_distance { 10 };        // gradient is measured over ±10 metres

            const double distance_m { distance_to_square - delta_distance };
            const double distance_p { distance_to_square + delta_distance };
          
            const pair<double, double> ll_m { ll_from_bd(qth, bearing_from_north, distance_m) };        
            const pair<double, double> ll_p { ll_from_bd(qth, bearing_from_north, distance_p) };        

            const float raw_value_m { tiles.at(llc(ll_m)).interpolated_value(ll_m) };                 // height per USGS
            const float raw_value_p { tiles.at(llc(ll_p)).interpolated_value(ll_p) };                 // height per USGS

            const double correction_m { curvature_correction(distance_m) };
            const double correction_p { curvature_correction(distance_p) };

            const double height_m { raw_value_m * cos(distance_m / RE) - correction_m };
            const double height_p { raw_value_p * cos(distance_p / RE) - correction_p };
          
            grad_field[row_index][column_index] = (height_p - height_m) / (2 * delta_distance);
          }
          
          catch (const grid_float_error& e)
          { cerr << "Caught grid float error while calculating grad field: " << e.reason() << endl;
          
            grad_field[row_index][column_index] = -9999;
          }
        }
      }
      
// visibility of this cell     
      if (los)
      { if (delta_x != 0 or delta_y != 0)                     // for everything except the QTH cell
        { const float angle { static_cast<float>(elev ? (elevation_angle_in_degrees * DTOR) : elevation_angle(qth, ll, raw_qth_height + antenna_height, raw_value)) }; 

          bool visible { true };
            
// walk along a bearing, looking to see if visibility is maintained
          int decrement { 1 };
            
// a bit of a fudge for very close-in terminating points
          if (distance_to_square < 250)               // 250m
            decrement = max(int(distance_to_square / 4), 1);  // ~25% per step
            
          try
          { for (int n = 95; visible and n >= 5; n -= decrement)                                            // skip points near ends to avoid rounding problems
            { const double               distance_to_square_n { (n * distance_to_square) / (100) };
              const pair<double, double> ll_n                 { ll_from_bd(qth, bearing_from_north, distance_to_square_n) };
              const float                raw_value_n          { tiles.at(llc(ll_n)).interpolated_value(ll_n) };
              const float                angle_n              { elevation_angle(qth, ll_n, raw_qth_height + antenna_height, raw_value_n) };              
              
             visible = (angle_n < angle);
            }
  
            { lock_guard<mutex> los_field_lock(los_field_mutex);                    // should not be necessary, but be paranoid

              los_field[row_index][column_index] = (visible ? VISIBILITY::VISIBLE : VISIBILITY::NOT_VISIBLE);
            }  
          }

          catch (...)  // default to NOT VISIBLE
          { cerr << "Exception handled when calculating LOS" << endl;
          
            lock_guard<mutex> los_field_lock(los_field_mutex);                    // should not be necessary, but be paranoid

            los_field[row_index][column_index] = VISIBILITY::NOT_VISIBLE;
          } 
        }
        else                                                  // QTH is always visible
        { lock_guard<mutex> los_field_lock(los_field_mutex);                    // should not be necessary, but be paranoid

          los_field[n_cells][n_cells] = VISIBILITY::VISIBLE;
        }
      }
    }
  }
}

/*! \brief                          Label the axes
    \param  R                       the R instance
    \param  distances_km            the values that are to be written
    \param  distances_in_metres     the distances where the values are to be written
    \param  long_distance_unit_str  the name of the distance unit to be written on the plot
*/
void label_axes(RInside& R, const vector<int>& distances_km, const vector<int>& distances_in_metres, const string& long_distance_unit_str)
{ vector<string> labels;
    
  for (const auto& d : distances_km)
    labels.push_back(to_string(d));

  execute_r(R, "par(mgp = c(3, 1.5, 0))");      // move tick labels to somewhere sensible; default 3,2,0; second number controls distance from axis to tick label

  label_ticks(R, 1, distances_in_metres, labels, true);
  label_ticks(R, 2, distances_in_metres, labels, true);
  r_function(R, "title", "xlab = '" + long_distance_unit_str + "', line = 1.5"s);
  r_function(R, "title", "ylab = '" + long_distance_unit_str + "', line = 1.5"s);
}

/*! \brief              Write the callsign, latitude and longitude information on the plot
    \param  R           the R instance
    \param  callsign    the value of the -call parameter
    \param  latitude    the latitude of the plot
    \param  longitude   the longitude of the plot
*/
void call_lat_long(RInside& R, const string& callsign, const double latitude, const double longitude)
{ string slashes { create_string(' ', callsign.size()) };
    
  for (size_t n = 0; n < callsign.size(); ++n)
    if (callsign[n] == '0')
      slashes[n] = '/';
   
  execute_r(R, "text(x=0.5, y = 0.65, labels = c('" + callsign + "'), cex = 2.3, font = 2, col = 'BLACK', family ='Noto Mono', adj = c(0.5, 0) )");  // basic call

  if (contains(callsign, '0'))
    execute_r(R, "text(x=0.5, y = 0.65, labels = c('" + slashes + "'), cex = 2.3, font = 2, col = 'BLACK', family ='Noto Mono', adj = c(0.5, 0) )");  // superimpose slashes on zeros

  const string latitude_str  { to_string(latitude) };
  const string lat_str       { substring(latitude_str, 0, 2) + "°·" + substring(latitude_str, 3, 5) + "N" };    
  const string longitude_str { to_string(-longitude) };
  const string long_str      { substring(longitude_str, 0, longitude <= -100 ? 3 : 2) + "°·" + substring(longitude_str, longitude <= -100 ? 4 : 3, 5) + "W" };

  execute_r(R, "text(x=0.50, y = 0.59, labels = c('" + lat_str + "'), cex = 1.5)");
  execute_r(R, "text(x=0.50, y = 0.55, labels = c('" + long_str + "'), cex = 1.5)");
}

/*! \brief                      Label the horizon gradient ( in the left side)
    \param  R                   the R instance
    \param  min_horizon         the minimum elevation of the horizon (rounded)
    \param  max_horizon         the maximum elevation of the horizon (rounded)
    \param  colour_gradient     colour gradient
*/
void label_horizon_gradient(RInside& R, const float min_horizon, const float max_horizon, r_colour_gradient& colour_gradient)
{ const vector<int> vi { r_seq(int(min_horizon), int(max_horizon), (int(max_horizon - min_horizon) / 10) + 1 ) };
    
  vector<string> vs;
     
  for (const auto& i : vi)
    vs.push_back(to_string(i));
     
  colour_gradient.l_label(vs);  

  constexpr float x { 0.85 }; 
  constexpr float y { 0.95 };
  
  execute_r(R, "text(x = "s + to_string(x) + ", y = "s + to_string(y) + ", labels = '"s + "Hzn\n(°)" + "', pos = 2)"s);
}
