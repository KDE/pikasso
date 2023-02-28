// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

extern crate lyon;

use lyon::path::{Path, geom::{point, vector}, builder::SvgPathBuilder, builder::WithSvg};
use lyon::math::Point;
use lyon::tessellation::*;
use lyon::path::path::Builder;
use lyon::tessellation::geometry_builder::simple_builder;

use ffi::{LyonPoint, LyonVector, LyonGeometry};

pub struct LyonBuilder {
    builder: WithSvg<Builder>,
}

impl LyonBuilder {
    fn close(&mut self) {
        self.builder.close();
    }

    fn move_to(&mut self, to: &LyonPoint) {
        self.builder.move_to(point(to.x, to.y));
    }

    fn line_to(&mut self, to: &LyonPoint) {
        self.builder.line_to(point(to.x, to.y));
    }

    fn quadratic_bezier_to(&mut self, ctrl: &LyonPoint, to: &LyonPoint) {
        self.builder.quadratic_bezier_to(point(ctrl.x, ctrl.y), point(to.x, to.y));
    }

    fn cubic_bezier_to(&mut self, ctrl1: &LyonPoint, ctrl2: &LyonPoint, to: &LyonPoint) {
        self.builder.cubic_bezier_to(point(ctrl1.x, ctrl1.y), point(ctrl2.x, ctrl2.y), point(to.x, to.y));
    }

    fn relative_move_to(&mut self, to: LyonVector) {
        self.builder.relative_move_to(vector(to.x, to.y));
    }
}

pub fn new_builder() -> Box<LyonBuilder> {
    return Box::new(LyonBuilder{
        builder: WithSvg::new(Path::builder())
    })
}

fn convert_points(old_points: Vec<Point>) -> Vec<LyonPoint> {
    let mut new_vec = Vec::with_capacity(old_points.len());

    for old_point in old_points {
        new_vec.push(LyonPoint{ x: old_point.x, y: old_point.y });
    }
    new_vec
}

pub fn build_fill(builder: Box<LyonBuilder>) -> LyonGeometry {
    let mut buffers: VertexBuffers<Point, u16> = VertexBuffers::new();
    {
        let mut vertex_builder = simple_builder(&mut buffers);

        // Create the tessellator.
        let mut tessellator = FillTessellator::new();

        let path = builder.builder.build();

        // Compute the tessellation.
        let result = tessellator.tessellate_path(
            &path,
            &FillOptions::tolerance(0.01),
            &mut vertex_builder
        );
        assert!(result.is_ok());
    }

    LyonGeometry {
        vertices: convert_points(buffers.vertices), // todo no convert
        indices: buffers.indices,
    }
}

pub fn build_stroke(builder: Box<LyonBuilder>, line_width: f32) -> LyonGeometry {
    let mut buffers: VertexBuffers<Point, u16> = VertexBuffers::new();
    {
        let mut vertex_builder = simple_builder(&mut buffers);

        // Create the tessellator.
        let mut tessellator = StrokeTessellator::new();

        let path = builder.builder.build();

        // Compute the tessellation.
        let result = tessellator.tessellate_path(
            &path,
            &StrokeOptions::tolerance(0.01).with_line_width(line_width),
            &mut vertex_builder
        );
        assert!(result.is_ok());
    }

    LyonGeometry {
        vertices: convert_points(buffers.vertices), // todo no convert
        indices: buffers.indices,
    }
}

#[cxx::bridge]
mod ffi {
    pub struct LyonPoint {
        x: f32,
        y: f32,
    }

    pub struct LyonVector {
        x: f32,
        y: f32,
    }

    pub struct LyonGeometry {
        vertices: Vec<LyonPoint>,
        indices: Vec<u16>,
    }

    extern "Rust" {
        type LyonBuilder;
        fn new_builder() -> Box<LyonBuilder>;
        fn move_to(self: &mut LyonBuilder, point: &LyonPoint);
        fn line_to(self: &mut LyonBuilder, point: &LyonPoint);
        fn relative_move_to(self: &mut LyonBuilder, to: LyonVector);
        fn close(self: &mut LyonBuilder);
        fn quadratic_bezier_to(self: &mut LyonBuilder, ctrl: &LyonPoint, to: &LyonPoint);
        fn cubic_bezier_to(self: &mut LyonBuilder, ctrl1: &LyonPoint, ctrl2: &LyonPoint, to: &LyonPoint);
        fn build_fill(builder: Box<LyonBuilder>) -> LyonGeometry;
        fn build_stroke(builder: Box<LyonBuilder>, line_width: f32) -> LyonGeometry;
    }
}

