/**
 * This file is part of the "FnordMetric" project
 *   Copyright (c) 2011-2014 Paul Asmuth, Google Inc.
 *
 * FnordMetric is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License v3.0. You should have received a
 * copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef _FNORDMETRIC_SVGTARGET_H
#define _FNORDMETRIC_SVGTARGET_H
#include <stdlib.h>
#include <vector>
#include <string>
#include <fnordmetric/ui/rendertarget.h>
#include <fnordmetric/ui/styles/style_default.h>
#include <fnordmetric/util/outputstream.h>

namespace fnordmetric {
namespace ui {

class SVGTarget : public RenderTarget {
public:

  /**
   * Create a new SVG target.
   *
   * @param output_stream the output stream to write to. does not transfer
   *                      ownership!
   */
  SVGTarget(
      util::OutputStream* output_stream) :
      output_(output_stream),
      indent_(0) {}

#define append(...) { output_->printf(__VA_ARGS__); }

#define appendLine(...) { \
    for(int __i = 0; __i < indent_ * 2; ++__i) append(" "); \
    append(__VA_ARGS__); }

  void beginChart(
      int width,
      int height,
      const std::string& class_name) {
    appendLine(
        "<svg viewBox='0 0 %i %i' class='%s'>\n",
        width,
        height,
        class_name.c_str());
    indent_ ++;
    appendLine("<style type='text/css'>\n");
    appendLine("<![CDATA[%s  ]]>\n", kStyleSheetDefault.c_str());
    appendLine("</style>\n");
  }

  void finishChart() {
    indent_--;
    appendLine("</svg>\n");
  }

  void drawRect(
      double x,
      double y,
      double width,
      double height,
      const std::string& color,
      const std::string& class_name) override {
    std::string class_str(class_name);
    class_str += " ";
    class_str += color;

    appendLine(
        "<rect x='%f' y='%f' width='%f' height='%f' class='%s'></rect>\n",
        x,
        y,
        width,
        height,
        class_str.c_str());
  }

  void drawLine(
      double x1,
      double y1,
      double x2,
      double y2,
      const std::string& class_name) override {
    appendLine(
        "<line x1='%f' y1='%f' x2='%f' y2='%f' class='%s' />\n",
        x1,
        y1,
        x2,
        y2,
        class_name.c_str());
  }

  void drawText(
      const std::string& text,
      double x,
      double y,
      const std::string& halign,
      const std::string& valign,
      const std::string& class_name,
      double rotate = 0.0f) override {
    char transform[256];

    if (rotate == 0.0f) {
      *transform = 0;
    } else {
      snprintf(
          transform,
          sizeof(transform),
          "transform='translate(0,0) rotate(%f %f %f)'",
          rotate,
          x,
          y);
    }

    appendLine(
        "<text x='%f' y='%f' style='text-anchor:%s; dominant-baseline:%s;' "
            "class='%s' %s>%s</text>\n",
        x,
        y,
        halign.c_str(),
        valign.c_str(),
        class_name.c_str(),
        transform,
        text.c_str());
  }

  void drawPoint(
      double x,
      double y,
      const std::string& point_type,
      double point_size,
      const std::string& color,
      const std::string& class_name /* = "" */,
      const std::string& label /* = "" */,
      const std::string& series /* = "" */) override {
    std::string class_str(class_name);
    class_str += " ";
    class_str += color;

    /* point_type: circle */
    // FIXPAUL escape label
    appendLine(
        "<circle cx='%f' cy='%f' r='%f' class='%s' fm:label='%s' "
            "fm:series='%s'></circle>\n",
        x,
        y,
        point_type == "none" ? 0 : point_size,
        class_str.c_str(),
        label.c_str(),
        series.c_str());
  }

   void drawPath(
      const std::vector<std::pair<double, double>>& points,
      const std::string& line_style,
      double line_width,
      bool smooth,
      const std::string& color,
      const std::string& class_name = "") override {
    std::string class_str(class_name);
    class_str += " ";
    class_str += color;

    appendLine(
        "<path stroke-width='%f' class='%s' d='",
        line_width,
        class_str.c_str());

    for (int i = 0; i < points.size(); ++i) {
      if (i == 0) {
        append("M%f %f ", points[i].first, points[i].second);
      } else if (smooth) {
        append("L%f %f ", points[i].first, points[i].second);
      } else {
        append("L%f %f ", points[i].first, points[i].second);
      }
    }

    append("' />\n");
  }

  void beginGroup(const std::string& class_name) override {
    appendLine("<g class='%s'>\n", class_name.c_str());
    indent_++;
  }

  void finishGroup() override {
    indent_--;
    appendLine("</g>\n");
  }

protected:
  util::OutputStream* output_;
  int indent_;
};


}
}
#endif
