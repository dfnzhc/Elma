Real light_power_op::operator()(const Envmap& light) const
{
    return kPi * scene.bounds.radius * scene.bounds.radius * light.sampling_dist.totalValues /
           (light.sampling_dist.width * light.sampling_dist.height);
}

PointAndNormal sample_point_on_light_op::operator()(const Envmap& light) const
{
    Vector2 uv = Sample(light.sampling_dist, rnd_param_uv);
    // Convert uv to spherical coordinates
    Real azimuth   = uv[0] * (2 * kPi);
    Real elevation = uv[1] * kPi;
    // Convert spherical coordinates to Cartesian coordinates.
    // (https://en.wikipedia.org/wiki/Spherical_coordinate_system#Cartesian_coordinates)
    // We use the convention that y is the up axis.
    Vector3 local_dir{sin(azimuth) * sin(elevation), cos(elevation), -cos(azimuth) * sin(elevation)};
    Vector3 world_dir = TransformVector(light.to_world, local_dir);
    return PointAndNormal{
      Vector3{0, 0, 0},
      -world_dir
    };
}

Real pdf_point_on_light_op::operator()(const Envmap& light) const
{
    // We store the direction pointing outwards from light in point_on_light.normal.
    Vector3 world_dir = -point_on_light.normal;
    // Convert the direction to local Catesian coordinates.
    Vector3 local_dir = TransformVector(light.to_local, world_dir);
    // Convert the Cartesian coordinates to the spherical coordinates.
    // We use the convention that y is the up-axis.
    Vector2 uv{atan2(local_dir[0], -local_dir[2]) * kInvTwoPi,
               acos(std::clamp(local_dir[1], Real(-1), Real(1))) * kInvPi};
    // atan2 returns -pi to pi, we map [-pi, 0] to [pi, 2pi]
    if (uv[0] < 0) {
        uv[0] += 1;
    }
    Real cos_elevation = local_dir.y;
    Real sin_elevation = std::sqrt(std::clamp(1 - cos_elevation * cos_elevation, Real(0), Real(1)));
    if (sin_elevation <= 0) {
        // degenerate
        return 0;
    }
    return Pdf(light.sampling_dist, uv) / (2 * kPi * kPi * sin_elevation);
}

Spectrum emission_op::operator()(const Envmap& light) const
{
    // View dir is pointing outwards "from" the light.
    // An environment map stores the light from the opposite direction,
    // so we need to flip view dir.
    // We then transform the direction to the local Cartesian coordinates.
    Vector3 local_dir = TransformVector(light.to_local, -view_dir);
    // Convert the Cartesian coordinates to the spherical coordinates.
    Vector2 uv{std::atan2(local_dir[0], -local_dir[2]) * kInvTwoPi,
               std::acos(std::clamp(local_dir[1], Real(-1), Real(1))) * kInvPi};
    // atan2 returns -pi to pi, we map [-pi, 0] to [pi, 2pi]
    if (uv[0] < 0) {
        uv[0] += 1;
    }

    // For envmap, view_footprint stores (approximatedly) d view_dir / dx
    // We want to convert it to du/dx -- we do it by computing (d dir / dx) * (d u / d dir)
    // To do this we differentiate through the process above:

    // We abbrevite local_dir as w
    Vector3 w  = local_dir;
    Real dudwx = -w.z / (w.x * w.x + w.z * w.z);
    Real dudwz = w.x / (w.x * w.x + w.z * w.z);
    Real dvdwy = -1 / std::sqrt(std::max(1 - w.y * w.y, Real(0)));
    // We only want to know the length of dudw & dvdw
    // The local coordinate transformation is length preserving,
    // so we don't need to differentiate through it.
    Real footprint = std::min(std::sqrt(dudwx * dudwx + dudwz * dudwz), dvdwy);

    return Eval(light.values, uv, footprint, scene.texturePool) * light.scale;
}

void InitSamplingDistOp::operator()(Envmap& light) const
{
    if (auto* t = std::get_if<ImageTexture<Spectrum>>(&light.values)) {
        // Only need to initialize sampling distribution
        // if the envmap is an image.
        const Mipmap3& mipmap = GetImage(*t, scene.texturePool);
        int w = GetWidth(mipmap), h = GetHeight(mipmap);
        std::vector<Real> f(w * h);
        int i = 0;
        for (int y = 0; y < h; y++) {
            // We shift the grids by 0.5 pixels because we are approximating
            // a piecewise bilinear distribution with a piecewise constant
            // distribution. This shifting is necessary to make the sampling
            // unbiased, as we can interpolate at a position of a black pixel
            // and get a non-zero contribution.
            Real v             = (y + Real(0.5)) / Real(h);
            Real sin_elevation = std::sin(kPi * v);
            for (int x = 0; x < w; x++) {
                Real u = (x + Real(0.5)) / Real(w);
                f[i++] = Luminance(Lookup(mipmap, u, v, 0)) * sin_elevation;
            }
        }
        light.sampling_dist = MakeTableDist2d(f, w, h);
    }
}
