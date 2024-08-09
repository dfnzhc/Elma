Vector2 SampleOp::operator()(const Gaussian& filter) const
{
    // Box Muller transform
    // https://en.wikipedia.org/wiki/Box%E2%80%93Muller_transform
    Real r = filter.stddev * sqrt(-2 * log(Max(rndParam[0], Real(1e-8))));
    return Vector2{r * cos(2 * kPi * rndParam[1]), r * sin(2 * kPi * rndParam[1])};
}
