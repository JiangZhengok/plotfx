/**
 * This file is part of the "libstx" project
 *   Copyright (c) 2011-2014 Paul Asmuth, Google Inc.
 *
 * libstx is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License v3.0. You should have received a
 * copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include "axes.h"
#include <assert.h>
#include <elements/context.h>
#include <elements/plot/plot_element.h>
#include <graphics/text.h>
#include <graphics/brush.h>

namespace signaltk {

AxisDefinition::AxisDefinition() :
    mode(AxisMode::DISABLED),
    label_placement(AxisLabelPlacement::OFF),
    label_padding_horiz_rem(kDefaultLabelPaddingHorizREM),
    label_padding_vert_rem(kDefaultLabelPaddingVertREM),
    tick_length_rem(kDefaultTickLengthREM) {}

Status plot_axis_add(context* ctx, AxisPosition pos) {
  PlotConfig* elem;
  if (auto rc = element_config_as(ctx, &elem); rc) {
    return rc;
  }

  auto axis_config = std::make_unique<AxisDefinition>();
  axis_config->position = pos;

  switch (pos) {
    case AxisPosition::LEFT:
      axis_config->label_placement = AxisLabelPlacement::LEFT;
      break;
    case AxisPosition::RIGHT:
      axis_config->label_placement = AxisLabelPlacement::RIGHT;
      break;
    case AxisPosition::TOP:
      axis_config->label_placement = AxisLabelPlacement::TOP;
      break;
    case AxisPosition::BOTTOM:
      axis_config->label_placement = AxisLabelPlacement::BOTTOM;
      break;
  }

  elem->axes.emplace_back(std::move(axis_config));
  return OK;
}

Status plot_axis_addtick(context* ctx, float offset) {
  PlotConfig* elem;
  if (auto rc = element_config_as(ctx, &elem); rc) {
    return rc;
  }

  auto& axis_config = elem->axes.back();
  axis_config->ticks.emplace_back(offset);
  return OK;
}

Status plot_axis_addlabel(context* ctx, float offset, const char* label) {
  PlotConfig* elem;
  if (auto rc = element_config_as(ctx, &elem); rc) {
    return rc;
  }

  auto& axis_config = elem->axes.back();
  axis_config->labels.emplace_back(offset, label);
  return OK;
}

Status plot_render_axis_vertical(
    const AxisDefinition& axis_config,
    double x,
    double y0,
    double y1,
    Layer* target) {
  /* draw axis line */ 
  {
    StrokeStyle style;
    strokeLine(target, x, y0, x, y1, style);
  }

  double label_placement = 0;
  switch (axis_config.label_placement) {
    case AxisLabelPlacement::RIGHT:
      label_placement = 1;
      break;
    case AxisLabelPlacement::LEFT:
      label_placement = -1;
      break;
    default:
      break;
  }

  /* draw ticks */
  for (const auto& tick : axis_config.ticks) {
    auto y = y0 + (y1 - y0) * tick;
    StrokeStyle style;
    strokeLine(
        target,
        x,
        y,
        x + from_rem(*target, axis_config.tick_length_rem) * label_placement,
        y,
        style);
  }

  /* draw labels */
  auto label_padding = from_rem(*target, axis_config.label_padding_horiz_rem);
  for (const auto& label : axis_config.labels) {
    auto [ tick, label_text ] = label;
    auto sy = y0 + (y1 - y0) * tick;
    auto sx = x + label_padding * label_placement;

    TextStyle style;
    style.halign = label_placement > 0 ? TextHAlign::LEFT : TextHAlign::RIGHT;
    style.valign = TextVAlign::MIDDLE;
    if (auto rc = drawText(label_text, sx, sy, style, target); rc != OK) {
      return rc;
    }
  }

  return OK;
}

Status plot_render_axis_horizontal(
    const AxisDefinition& axis_config,
    double y,
    double x0,
    double x1,
    Layer* target) {
  /* draw axis line */ 
  {
    StrokeStyle style;
    strokeLine(target, x0, y, x1, y, style);
  }

  double label_placement = 0;
  switch (axis_config.label_placement) {
    case AxisLabelPlacement::BOTTOM:
      label_placement = 1;
      break;
    case AxisLabelPlacement::TOP:
      label_placement = -1;
      break;
    default:
      break;
  }

  /* draw ticks */
  for (const auto& tick : axis_config.ticks) {
    auto x = x0 + (x1 - x0) * tick;
    StrokeStyle style;
    strokeLine(
        target,
        x,
        y,
        x,
        y + from_rem(*target, axis_config.tick_length_rem) * label_placement,
        style);
  }

  /* draw labels */
  auto label_padding = from_rem(*target, axis_config.label_padding_vert_rem);
  for (const auto& label : axis_config.labels) {
    auto [ tick, label_text ] = label;
    auto sx = x0 + (x1 - x0) * tick;
    auto sy = y + label_padding * label_placement;

    TextStyle style;
    style.halign = TextHAlign::CENTER;
    style.valign = label_placement > 0 ? TextVAlign::TOP : TextVAlign::BOTTOM;
    if (auto rc = drawText(label_text, sx, sy, style, target); rc) {
      return rc;
    }
  }

  return OK;
}

Status plot_render_axis(context* ctx, int i) {
  const PlotConfig* elem;
  if (auto rc = element_config_as(ctx, &elem); rc) {
    return rc;
  }

  assert(i < elem->axes.size());

  int padding = 80;
  const auto& axis = elem->axes[i];

  Status rc;
  switch (axis->position) {
    case AxisPosition::LEFT:
      rc = plot_render_axis_vertical(
          *axis,
          padding,
          padding,
          context_height(ctx) - padding,
          context_frame(ctx));
      break;
    case AxisPosition::RIGHT:
      rc = plot_render_axis_vertical(
          *axis,
          context_width(ctx) - padding,
          padding,
          context_height(ctx) - padding,
          context_frame(ctx));
      break;
    case AxisPosition::TOP:
      rc = plot_render_axis_horizontal(
          *axis,
          padding,
          padding,
          context_width(ctx) - padding,
          context_frame(ctx));
      break;
    case AxisPosition::BOTTOM:
      rc = plot_render_axis_horizontal(
          *axis,
          context_height(ctx) - padding,
          padding,
          context_width(ctx) - padding,
          context_frame(ctx));
      break;
  }

  return rc;
}


}