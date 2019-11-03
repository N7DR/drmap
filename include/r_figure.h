// $Id: r_figure.h 15 2019-11-03 15:02:05Z n7dr $

// Released under the GNU Public License, version 2

// Principal author: N7DR

// Copyright owners:
//    N7DR

/*! \file   r_figure.h

    Classes and functions related to R figures
*/

#ifndef R_FIGURE_H
#define R_FIGURE_H

#include "string_functions.h"

#include <RInside.h>

#include <experimental/string_view>

constexpr int N_GRADIENT_COLOURS { 1000 };  ///< default number of colours in a gradient

constexpr bool DRAW_TICKS    { true },
               NO_DRAW_TICKS { false };

extern bool TRACE_R;                ///< whether to output commands sent through <i>execute_r()</i>

/*! \brief          Execute, verbatim, a command in R
    \param  R       the R instance
    \param  cmd     the command to execute

    Copies the command to <i>cut</i> if <i>TRACE_R</i> is <i>true</i>
*/
void execute_r(RInside& R, std::experimental::string_view cmd);

/*! \brief              Initialise a PNG file
    \param  R           the R instance
    \param  filename    name of file to hold the plot
    \param  w           width of figure, in pixels
    \param  h           height of figure, in pixels
*/
inline void create_figure(RInside& R, const std::string& filename, const unsigned int w = 800, const unsigned int h = 600)
  { execute_r(R, "png(filename='"s + filename + "',  width="s + to_string(w) + ", height="s + to_string(h) + ")"s); }

/*! \brief                      Create a number of screens on a figure
    \param  R                   the R instance
    \param  screen_definitions  the corners of the screens: XMIN, XMAX, YMIN, YMAX

    sends commands like:
      screen_1 <- c(0.0, 0.9, 0.0, 1.0)
      screen_2 <- c(0.9, 1.0, 0.0, 1.0)
    then
      executes split.screen()
*/
void create_screens(RInside& R, const std::vector< std::array<float, 4>>& screen_definitions);

/*! \brief      Select a particular screen for plotting commands that follow
    \param  R   the R instance
    \param  n   the screen number
*/
inline void select_screen(RInside& R, const int n)
  { execute_r(R, "screen("s + to_string(n) + ", new = FALSE)"s + EOL); }

/*! \brief      Send a plot() command to R
    \param  R   the R instance
*/
template<typename X, typename Y>
void start_plot(RInside& R, X xmin = 0, X xmax = 1, Y ymin = 0, Y ymax = 1)
{ execute_r(R, "plot(0, 0, axes = F, type = 'n', xlim = c("s + to_string(xmin) + ", "s + to_string(xmax) +
                 "), ylim = c("s + to_string(ymin) + ", "s + to_string(ymax) + "), xaxt = 'n', yaxt = 'n', xlab = '', ylab = '', main = '')"s);
}

/*! \brief          Set the colour of the plot rectangle
    \param  R       the R instance
    \param  clr     the colour of the rectangle
*/
inline void set_rect(RInside& R, const std::string& clr)
  { execute_r(R, "rect(par(\"usr\")[1], par(\"usr\")[3], par(\"usr\")[2], par(\"usr\")[4], col = '"s + clr + "')"s ); }

/*! \brief              Draw a horizontal line
    \param  R           the R instance
    \param  y_value     the y value at which to draw the line
    \param  x_value_1   the x value at one end of the line
    \param  x_value_2   the x value at the other end of the line
*/
template<typename Y, typename X>
void draw_horizontal_line(RInside& R, const Y y_value, const X x_value_1, const X x_value_2)
{ const std::string x_str { "c(" + to_string(x_value_1) + "," + to_string(x_value_2) + ")" };
  const std::string y_str { "c(" + to_string(y_value) + "," + to_string(y_value) + ")" };

  std::string cmd { "lines( " + x_str + ", " + y_str + ")" };
  
  execute_r(R, cmd);
}

//    legend(x = xpos, y = ypos, legend = c('Total', '160m', '80m', '40m', '20m', '15m', '10m'),
//           lty=c(1, 1), lwd=c(2,2), col = legend_colours,
//           bty = 'n', text.col = 'black')
void write_legend(RInside& R, const float x, const float y, const std::vector<std::string>& legends, const std::vector<std::string>& colours);

template<typename T>
const std::string c(const std::vector<T>& strs)
{ std::string rv { "c("s };

  for (size_t n = 0; n < strs.size(); ++n)
  { rv += "'"s + to_string(strs[n]) + "'"s;

    if (n != strs.size() - 1)
      rv += ", "s;
  }

  rv += ")"s;

  return rv;
}

//const std::string c(const std::vector<std::string>& strs);

/*! \brief          Title the figure
    \param  R       the R instance
    \param  str     the title for the figure
*/
inline void set_title(RInside& R, const std::string& str)
  { execute_r(R, "title('"s + str + "')"s ); }

/*! \brief          Send a function with parameters to R
    \param  R       the R instance
    \param  fname   function name
    \param  fparams the parameter string
*/
inline void r_function(RInside& R, const std::string& fname, const std::string& fparams)
  { execute_r(R, fname + "("s + fparams + ")"s ); }

/*! \brief          Create a vector of values covering a range
    \param  start   first value
    \param  final   last value
    \param  by      amount by which to increment from one element to the next

    This version assumes that <i>by</i> is positive
*/
//const std::vector<int> r_seq(const int start = 1, const int final = 1, const int by = 1);

template<typename T>
const std::vector<T> r_seq(const T start = 1, const T final = 1, const T by = 1)
{ //std::cout << "r_seq: " << start << ", " << final << ", " << by << std::endl;
  
  std::vector<T> rv;

  for (auto n = start; n <= final; n += by)
    rv.push_back(n);

  return rv;
}

/*! \brief      Control tracing of commands executed by <i>execute_r()</i>
    \param  b   whether to trace
*/
inline void trace_r(const bool b)
  { TRACE_R = b; }

/*! \brief          Display the N7DR logo
    \param  R       the R instance
*/
void display_logo(RInside& R);

// -----------  r_colour_gradient  ----------------

/*! \class  r_colour_gradient
    \brief  Class to encapsulate an R colour gradient
*/

class r_colour_gradient
{
protected:

  float         _gradient_bottom { 0.1 };           ///< location of bottom of gradient strip
  int           _gradient_nr;                       ///< global gradient number
  std::string   _gradient_name;                     ///< the R name for the gradient
  float         _gradient_top { 0.9 };              ///< location of top of gradient strip
  int           _n_colours { N_GRADIENT_COLOURS };  ///< number of distinct colours in the gradient (default = 1000)
  RInside&      _R;                                 ///< the internal R instance

public:

/*! \brief          Constructor
    \param  R       the R instance
    \param  clrs    the colours that define the gradient

    The colours may be either names or "#RRGGBB" colour definitions
*/
  r_colour_gradient(RInside& R, const std::vector<std::string>& clrs);

  ~r_colour_gradient(void);

  READ_AND_WRITE(gradient_bottom);          ///< location of bottom of gradient strip
  READ_AND_WRITE(gradient_top);             ///< location of top of gradient strip
  READ_AND_WRITE(n_colours);                ///< number of distinct colours in the gradient (default = 1000)

/*! \brief                  Create an R colour vector that corresponds to the gradient
    \param  vector_name     the name of the resulting R vector
*/
  inline void create_colour_vector(const std::string& vector_name)
    { execute_r(_R, vector_name + " <- "s + _gradient_name + "("s + to_string(_n_colours) + ")"s); };

/*! \brief  Display the gradient
*/
  void display(void);

/*! \brief          Apply labels to the right hand side of the gradient
    \param  lbls    the labels to apply
*/
  void label(const std::vector<std::string>& lbls);

/*! \brief          Apply labels to the left hand side of the gradient
    \param  lbls    the labels to apply
*/
  void l_label(const std::vector<std::string>& lbls);

/*! \brief          Apply labels to the right hand side of the gradient
    \param  lbls    the labels to apply
*/
  void label(const std::vector<int>& int_lbls);

/*! \brief          The title for the gradient
    \param  lbl     the title
    \param  x       x-coord (within the screen)
    \param  t       y-coord (within the screen)
*/
  inline void title(const std::string& lbl, const float x = 0.5, const float y = 0.925)
    { execute_r(_R, "text(x = "s + to_string(x) + ", y = "s + to_string(y) + ", labels = '"s + lbl + "')"s); };

/*! \brief                  Create a vector of labels for the gradient
    \param  max_value       highest value on the gradient
    \param  prepend_zero    whether to include a zero at the lower end
    \return                 a vector containing the labels for the gardient
*/
  template <class T>
    const std::vector<std::string> labels(const T max_value, const bool prepend_zero = false)
    { std::vector<std::string> rv;

      auto values = [=] (const int n_values)
        { std::vector<std::string> rv;

          const float increment { static_cast<float>(max_value) / n_values };

          if (prepend_zero)
            rv.push_back("0"s);

          for (auto n = 1; n <= n_values; ++n)
            rv.push_back(to_string(n * increment));

          return rv;
        };

      const auto&       diff        { max_value };
      const std::string diff_str    { to_string(diff) };
      const auto        posn        { diff_str.find_first_of("123456789") };
      const char        first_digit { diff_str[posn] };

//      char first_digit = to_string(max_value)[0];

      switch (first_digit)
      { case '1' :
          return values(10);

        case '2' :
          return values(4);

        case '3' :
          return values(6);

        case '4' :
          return values(4);

        case '5' :
          return values(5);

        case '6' :
          return values(6);

        case '7' :
          return values(7);

        case '8' :
          return values(8);

        case '9' :
          return values(9);

        default :               // should never get here
          break;
      }

      return rv;    // should never get here
    }

/*! \brief                  Create a vector of labels for the gradient, prepended with a particular string
    \param  prepend_string  string to be prepended to each label
    \param  max_value       highest value on the gradient
    \param  prepend_zero    whether to include a zero at the lower end
    \return                 a vector containing the labels for the gradient
*/
  template <class T>
    const std::vector<std::string> labels(const std::string& prepend_str, const T max_value, const bool prepend_zero = false)
    { std::vector<std::string> rv { labels(max_value, prepend_zero) };

      FOR_ALL(rv, [=] (std::string& str) { str = prepend_str + str;  } );  // exponentiate

      return rv;
    }

/*! \brief                  Create a vector of labels for the gradient
    \param  min_value       lowest value on the gradient
    \param  max_value       highest value on the gradient
    \return                 a vector containing the labels for the gradient
*/
  template <class T>
    const std::vector<std::string> labels(const T min_value, const T max_value)
    { std::vector<std::string> rv;

      auto values = [=] (const int n_values)
        { std::vector<std::string> rv;

          const float increment { static_cast<float>(max_value - min_value) / n_values };

          for (auto n = 0; n <= n_values; ++n)
            rv.push_back(to_string(min_value + (n * increment)));

          return rv;
        };

      const auto        diff        { max_value - min_value };
      const std::string diff_str    { to_string(diff) };
      const auto        posn        { diff_str.find_first_of("123456789") };
      const char        first_digit { diff_str[posn] };

      switch (first_digit)
      { case '1' :
          return values(10);

        case '2' :
          return values(4);

        case '3' :
          return values(6);

        case '4' :
          return values(4);

        case '5' :
          return values(5);

        case '6' :
          return values(6);

        case '7' :
          return values(7);

        case '8' :
          return values(8);

        case '9' :
          return values(9);

        default :               // should never get here
          break;
      }

      return rv;    // should never get here
    }

/*! \brief                      Display the gradient, including the surrounding material, on the second screen
    \param  gradient_title      the title for the gradient
    \param  gradient_lables     the vector of labels for the right-hand side of the gradient
*/
  void display_all_on_second_screen(const std::string& gradient_title, const std::vector<std::string>& gradient_labels);

/*! \brief      Return the vector of strings that represent the colours in the gradient
    \return     the colours in the gradient
*/
  const std::vector<std::string> colour_vector(void);

/*! \brief                      Return the vector of strings that represent the colours in the gradient, with an initial element prepended
    \param  initial_element     the element to be prepended
    \return                     the colours in the gradient, with <i>initial_element</i> prepended
*/
  const std::vector<std::string> colour_vector_with_initial_element(const std::string& initial_element);
};

// -----------  r_rect  ----------------

/*! \class  r_rect
    \brief  Class to encapsulate an R rect
*/

class r_rect
{
protected:

  float         _xl;                                ///< left x coordinate
  float         _xr;                                ///< right x coordinate
  float         _yb;                                ///< bottom y coordinate
  float         _yt;                                ///< top y coordinate
  std::string   _fill_colour;                       ///< name of colour to fill the rectangle
  std::string   _border_colour;
  RInside&      _R;                                 ///< the internal R instance

  bool          _no_border { false };

public:

  explicit inline r_rect(RInside& R) :
    _R(R)
  { }

  inline r_rect(RInside& R, float x1, float x2, float y1, float y2) :
    _xl(x1),
    _xr(x2),
    _yb(y1),
    _yt(y2),
    _R(R)
  { }

  inline r_rect(RInside& R, float x1, float x2, float y1, float y2, const std::string& fcolour) :
    _xl(x1),
    _xr(x2),
    _yb(y1),
    _yt(y2),
    _R(R),
    _fill_colour(fcolour)
  { }

  inline r_rect(RInside& R, float x1, float x2, float y1, float y2, const std::string& fcolour, const std::string& bcolour) :
    _xl(x1),
    _xr(x2),
    _yb(y1),
    _yt(y2),
    _R(R),
    _fill_colour(fcolour)
  { if (bcolour == "NO BORDER"s)
    { _border_colour.clear();
      _no_border = true;
    }
    else
      _border_colour = bcolour;
  }

  READ_AND_WRITE(xl);
  READ_AND_WRITE(xr);
  READ_AND_WRITE(yb);
  READ_AND_WRITE(yt);
  READ_AND_WRITE(fill_colour);
  READ_AND_WRITE(border_colour);
  READ_AND_WRITE(no_border);

  void draw(void) const;
};

// -----------  r_rects  ----------------

/*! \class  r_rects
    \brief  Class to encapsulate multiple R rects
*/

template <class T>
class r_rects
{
protected:

  std::vector<T>  _xl, _xr, _yb, _yt;   ///< the corners of the cells
      
  std::vector<std::string> _clr;        ///< the colours of the cells

  RInside&      _R;                     ///< the internal R instance

public:

  r_rects(RInside& R, const size_t size = 0) :
    _R(R)
  { if (size)
    { _xl.reserve(size);
      _xr.reserve(size);
      _yb.reserve(size);
      _yt.reserve(size);
      _clr.reserve(size);
    }
      
  }
  
  void add(const T new_xl, const T new_xr, const T new_yb, const T new_yt, const std::string& new_clr)
  { _xl.push_back(new_xl);
    _xr.push_back(new_xr);
    _yb.push_back(new_yb);
    _yt.push_back(new_yt);
    _clr.push_back(new_clr);
  }
  
  void draw(void) const
  { _R["rects_xl"] = _xl;
    _R["rects_xr"] = _xr;
    _R["rects_yb"] = _yb;
    _R["rects_yt"] = _yt;
    _R["rects_clr"] = _clr;

    execute_r(_R, "rect(xl = rects_xl, xr = rects_xr, yb = rects_yb, yt = rects_yt, col = rects_clr, border = NA)");

    execute_r(_R, "rects_xl <- NULL");
    execute_r(_R, "rects_xr <- NULL");
    execute_r(_R, "rects_yb <- NULL");
    execute_r(_R, "rects_yt <- NULL");
    execute_r(_R, "rects_clr <- NULL");
  }
};

// -----------  value_map  ----------------

/*! \class  value_map
    \brief  Class to encapsulate mapping from a domain to a range
*/

template<typename D, typename R>
class value_map
{
protected:

  D         _min_domain;
  D         _max_domain;
  R         _min_range;
  R         _max_range;

public:

  value_map(D d1, D d2, R r1, R r2) :
    _min_domain(d1),
    _max_domain(d2),
    _min_range(r1),
    _max_range(r2)
  { }

  R map_value(D d) const
  { const double factor { (static_cast<double>(d) - _min_domain) / (static_cast<double>(_max_domain) - _min_domain) };
    const double v      { factor * (static_cast<double>(_max_range) - _min_range) + _min_range };

    R rv { static_cast<R>(v) };  // but really we want round, not floor

    return rv;
  }

  inline R operator()(D d) const
    { return map_value(d); }
};

// -----------  zero_value_map  ----------------

/*! \class  zero_value_map
    \brief  Class to encapsulate mapping from a domain to a range, with zero treated specially
*/

template<typename D, typename R>
class zero_value_map : public value_map<D, R>
{
protected:

  R         _zero_maps_to;

public:

  zero_value_map(D d1, D d2, R r1, R r2, R zero) :
    value_map<D, R>(d1, d2, r1, r2),
    _zero_maps_to(zero)
  { }

  R map_value(D d) const
  { if (d == 0)
      return _zero_maps_to;

    return value_map<D, R>::map_value(d);
  }
};

/*! \brief      Round upwards to the next higher sensible number
    \param  x   number to round
    \return     <i>x</i> rounded upward to what should be a sensible number
*/
const double auto_round(const double x);

template <typename T, typename L>
void label_ticks(RInside& R, const int side_nr, const std::vector<T>& side_labels_at, const std::vector<L>& side_labels, const bool draw_ticks = true)
{ const pid_t pid = getpid();
  const std::string sla = "side_labels_at_"s + to_string(pid);
  const std::string sl = "side_labels_"s + to_string(pid);

  R[sla] = side_labels_at;
  R[sl] = side_labels;
  
#if 0
  int index = 0;
  
  for (const auto& a : side_labels_at)
    std::cout << index++ << ": " << a << std::endl;
    
  index = 0;
  
  for (const auto& b: side_labels)
    std::cout << index++ << ": " << b << std::endl;
#endif

  if (draw_ticks)
    execute_r(R, "axis(side = "s + to_string(side_nr) + ", at = "s + sla + ", labels = FALSE, las = 1, lwd = 0, lwd.ticks = 1)"s);    // draw ticks

//  if (side_labels_at.size() <= 20)
    execute_r(R, "axis(side = "s + to_string(side_nr) + ", at = "s + sla + ", labels = "s + sl +
               ", las = 1, lwd = 0, lwd.ticks = 0, line = -0.85, tick = FALSE)"s);  // labels
//  else
//    execute_r(R, "staxlab(side = "s + to_string(side_nr) + ", at = "s + sla + ", labels = "s + sl +
//               ", las = 1, lwd.ticks = 0, line = -0.85, tick = FALSE)"s);  // labels

  execute_r(R, sla + " <- NULL"s);
  execute_r(R, sl + " <- NULL"s);
}

template <typename T, typename L>
void label_ticks(RInside& R, const int side_nr, const std::vector<T>& side_labels_at, const std::vector<L>& side_labels, const bool draw_ticks, const float padj)
{ const pid_t pid = getpid();
  const std::string sla = "side_labels_at_"s + to_string(pid);
  const std::string sl = "side_labels_"s + to_string(pid);

  R[sla] = side_labels_at;
  R[sl] = side_labels;

  if (draw_ticks)
    execute_r(R, "axis(side = "s + to_string(side_nr) + ", at = "s + sla + ", labels = FALSE, las = 1, lwd = 0, lwd.ticks = 1)"s);    // draw ticks

  execute_r(R, "axis(side = "s + to_string(side_nr) + ", at = "s + sla + ", labels = "s + sl +
               ", las = 1, lwd = 0, lwd.ticks = 0, line = -0.85, tick = FALSE, padj = "s + to_string(padj) + ")"s);  // labels

//  execute_r(R, "axis(side = "s + to_string(side_nr) + ", at = "s + sla + ", labels = "s + sl +
//               ", las = 1, lwd = 0, lwd.ticks = 0, line = -0.50, tick = FALSE, padj = "s + to_string(padj) + ")"s);  // labels


  execute_r(R, sla + " <- NULL"s);
  execute_r(R, sl + " <- NULL"s);
}

#endif /* R_FIGURE_H */
