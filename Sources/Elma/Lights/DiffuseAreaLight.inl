Real light_power_op::operator()(const DiffuseAreaLight& light) const
{
    return Luminance(light.intensity) * SurfaceArea(scene.shapes[light.shape_id]) * kPi;
}

PointAndNormal sample_point_on_light_op::operator()(const DiffuseAreaLight& light) const
{
    const Shape& shape = scene.shapes[light.shape_id];
    return SamplePointOnShape(shape, ref_point, rnd_param_uv, rnd_param_w);
}

Real pdf_point_on_light_op::operator()(const DiffuseAreaLight& light) const
{
    return PdfPointOnShape(scene.shapes[light.shape_id], point_on_light, ref_point);
}

Spectrum emission_op::operator()(const DiffuseAreaLight& light) const
{
    if (Dot(point_on_light.normal, view_dir) <= 0) {
        return MakeZeroSpectrum();
    }
    return light.intensity;
}

void InitSamplingDistOp::operator()(DiffuseAreaLight& light) const
{
}
