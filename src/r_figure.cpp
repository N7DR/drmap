// $Id: r_figure.cpp 15 2019-11-03 15:02:05Z n7dr $

// Released under the GNU Public License, version 2
//   see: https://www.gnu.org/licenses/gpl-2.0.html

// Principal author: N7DR

// Copyright owners:
//    N7DR

/*! \file   r_figure.cpp

    Classes and functions related to R figures
*/

#include "r_figure.h"

#include <experimental/string_view>

using namespace std;

int  NEXT_GRADIENT_NUMBER { 0 };            ///< in oder to separate different gradients
bool TRACE_R              { false };        ///< do not output commands sent through <i>execute_r()</i>

extern bool debug;

/*! \brief                      Create a number of screens on a figure
    \param  R                   the R instance
    \param  screen_definitions  the corners of the screens: XMIN, XMAX, YMIN, YMAX

    sends commands like:
      screen_1 <- c(0.0, 0.9, 0.0, 1.0)
      screen_2 <- c(0.9, 1.0, 0.0, 1.0)
    then
      executes split.screen()
*/
void create_screens(RInside& R, const std::vector< std::array<float, 4>>& screen_definitions)
{ string cmd;

  const size_t n_screens { screen_definitions.size() };

  for (size_t n_screen = 1; n_screen <= n_screens; ++n_screen)
  { const string screen_name { "screen_"s + to_string(n_screen) };

    string this_cmd { screen_name + " <- c("s };

    const string definitions_string { join(screen_definitions[n_screen - 1], ","s) };

    this_cmd += ( definitions_string + ") ;"s );
    cmd += this_cmd;
  }

  execute_r(R, cmd);
  
  if (n_screens == 2)
    execute_r(R, "screen_numbers <- split.screen(matrix( c(screen_1, screen_2), byrow = T, ncol = 4), erase = FALSE)"s);    // assume just two screens for now
    
  if (n_screens == 3)
    execute_r(R, "screen_numbers <- split.screen(matrix( c(screen_1, screen_2, screen_3), byrow = T, ncol = 4), erase = FALSE)"s);    // assume just two screens for now
}

/*! \brief          Execute, verbatim, a command in R
    \param  R       the R instance
    \param  cmd     the command to execute

    Copies the command to <i>cout</i> if <i>TRACE_R</i> is <i>true</i>
*/
void execute_r(RInside& R, experimental::string_view cmd)
{ if (TRACE_R)
    cout << "R cmd: " << cmd << endl;

  R.parseEval(static_cast<string>(cmd));
}

/*! \brief          Display the N7DR logo
    \param  R       the R instance
*/
void display_logo(RInside& R)
{ execute_r(R, "text(x = 0.85, y = 0.025, labels = c('N7DR'))"s);
  execute_r(R, "rect(xl = 0.15, yb = 0.01, xr = 1.50, yt = 0.04, col = NA, lwd = 2)"s);
}

//    legend(x = xpos, y = ypos, legend = c('Total', '160m', '80m', '40m', '20m', '15m', '10m'),
//           lty=c(1, 1), lwd=c(2,2), col = legend_colours,
//           bty = 'n', text.col = 'black')
void write_legend(RInside& R, const float x, const float y, const vector<string>& legends, const std::vector<string>& colours)
{ string cmd = "legend(x = " + to_string(x) + ", y = " + to_string(y) + ", legend = " + c(legends) +
                       ", col = " + c(colours) +  ", lty=c(1, 1)" + ", lwd=c(2,2)" /*, bty = 'n', text.col = 'black'" + */ ")";

  execute_r(R, cmd);
}


// -----------  r_colour_gradient  ----------------

/*! \class  r_colour_gradient
    \brief  Class to encapsulate an R colour gradient
*/

/*! \brief          Constructor
    \param  R       the R instance
    \param  clrs    the colours that define the gradient

    The colours may be either names or "#RRGGBB" colour definitions
*/
r_colour_gradient::r_colour_gradient(RInside& R, const vector<string>& clrs) :
  _gradient_nr(NEXT_GRADIENT_NUMBER++),
  _gradient_name("GRADIENT_"s + to_string(_gradient_nr)),
  _R(R)
{ string cmd;

  for (size_t n = 0; n < clrs.size(); ++n)
  { cmd += "'"s + clrs[n] +"'"s;

    if (n != clrs.size() - 1)
      cmd += ", "s;
  }

  cmd = "colorRampPalette(c("s + cmd + "))"s;
  cmd = _gradient_name + " <- "s + cmd;

  execute_r(_R, cmd);            // execute command to create GRADIENT_# in R
}

r_colour_gradient::~r_colour_gradient(void)
{ const string cmd { "GRADIENT_"s + to_string(_gradient_nr) + "<- NULL"s };

  execute_r(_R, cmd);            // execute command to delete GRADIENT_# in R
}

/*! \brief  Display the gradient
*/
void r_colour_gradient::display(void)
{ string cmd = "rasterImage("s +
               "as.raster(matrix("s + _gradient_name + "("s + to_string(_n_colours) + "), "s + "ncol = 1)), "s +
               to_string(_gradient_top) + ", "s + to_string(_gradient_top) + ", "s +
               to_string(_gradient_top - 2 * _gradient_bottom) + ", "s + to_string(_gradient_bottom) + ", angle = 0)"s;

  execute_r(_R, cmd);
}

/*! \brief          Apply labels to the right hand side of the gradient
    \param  lbls    the labels to apply
*/
void r_colour_gradient::label(const vector<string>& lbls)
{ const auto n_labels { lbls.size() };

 // cout << "n_labels = " << n_labels << endl;

  const float delta_y = (_gradient_top - _gradient_bottom) / (n_labels - 1);

  _R["y_labels"s] = lbls;

  execute_r(_R, "y_labels_at <- seq("s + to_string(_gradient_bottom) +
                ", by = "s + to_string(delta_y) + ", length.out = "s + to_string(n_labels) + ")"s);

  execute_r(_R, "text(x=0.80, y = y_labels_at, labels = y_labels, pos = 4)"s);
}

/*! \brief          Apply labels to the right hand side of the gradient
    \param  lbls    the labels to apply
*/
void r_colour_gradient::label(const std::vector<int>& int_lbls)
{ vector<string> vs;

  FOR_ALL(int_lbls, [&vs] (const int& lbl) { vs.push_back(to_string(lbl)); } );

  label(vs);
}

/*! \brief          Apply labels to the left hand side of the gradient
    \param  lbls    the labels to apply
*/
void r_colour_gradient::l_label(const vector<string>& lbls)
{ const auto n_labels { lbls.size() };
  const float delta_y = (_gradient_top - _gradient_bottom) / (n_labels - 1);

  _R["y_labels"s] = lbls;

  execute_r(_R, "y_labels_at <- seq("s + to_string(_gradient_bottom) +
                ", by = "s + to_string(delta_y) + ", length.out = "s + to_string(n_labels) + ")"s);

  execute_r(_R, "text(x=0.85, y = y_labels_at, labels = y_labels, pos = 2)"s);  // pos=2 => right justify
}

/*! \brief                      Display the gradient, including the surrounding material, on the second screen
    \param  gradient_title      the title for the gradient
    \param  gradient_lables     the vector of labels for the right-hand side of the gradient
*/
void r_colour_gradient::display_all_on_second_screen(const string& gradient_title, const vector<string>& gradient_labels)
{ select_screen(_R, 2);

  r_function(_R, "par", "mar = rep(0, 4)"s);

  start_plot<int, int>(_R, 0, 2);
  
  constexpr float x { 0.8 }; 
  constexpr float y { 0.95 };
  
  execute_r(_R, "text(x = "s + to_string(x) + ", y = "s + to_string(y) + ", labels = '"s + gradient_title + "', pos = 4)"s);

  display();

  label( { gradient_labels } );
}

/*! \brief      Return the vector of strings that represent the colours in the gradient
    \return     the colours in the gradient
*/
const vector<string> r_colour_gradient::colour_vector(void)
{ const pid_t pid         { getpid() };
  const string r_variable { "colour_vector_"s + to_string(pid) };

  create_colour_vector(r_variable);

  vector<string> rv = move(_R[r_variable]);             // initialiser-list rv { std::move(_R[r_variable]) } causes crash

  execute_r(_R, r_variable + " <- NULL"s);  // remove the variable from R

  return rv;
}

/*! \brief                      Return the vector of strings that represent the colours in the gradient, with an initial element prepended
    \param  initial_element     the element to be prepended
    \return                     the colours in the gradient, with <i>initial_element</i> prepended
*/
const vector<string> r_colour_gradient::colour_vector_with_initial_element(const std::string& initial_element)
{ const vector<string> dv2 { colour_vector() };

  vector<string> rv { initial_element };

  rv.insert( rv.end(), dv2.cbegin(), dv2.cend() );

  return rv;
}

// -----------  r_rect  ----------------

/*! \class  r_rect
    \brief  Class to encapsulate an R rect
*/
//  execute_r(R, "rect(xl, yb, xr, yt, col = 'white', border = 'black')");
void r_rect::draw(void) const
{ string params { (to_string(_xl) + ", "s) };

  params += (to_string(_yb) + ", "s);
  params += (to_string(_xr) + ", "s);
  params += to_string(_yt);

  if (!_fill_colour.empty())
  { if (starts_with(_fill_colour, "{"s))
    { string clr { delimited_substring(_fill_colour, '{', '}') };  // e.g., {cs[v]}

      params += (", col = "s + clr);
    }
    else
      params += (", col = '"s + _fill_colour +"'"s);
  }

  if (_no_border)
    params += (", border = NA"s);
  else
  { if (!_border_colour.empty())
      params += (", border = '"s + _border_colour +"'"s);
  }

  r_function(_R, "rect"s, params);
}

/*! \brief      Round upwards to the next higher sensible number
    \param  x   number to round
    \return     <i>x</i> rounded upward to what should be a sensible number
*/
const double auto_round(const double x)
{ const double lg { log10(x) };

  if (lg == static_cast<int>(lg))
    return x;

  const int q    { static_cast<int>(lg) };
  const int fact { static_cast<int>(x / pow(10, q)) + 1 };

  return (fact * pow(10, q));
}
