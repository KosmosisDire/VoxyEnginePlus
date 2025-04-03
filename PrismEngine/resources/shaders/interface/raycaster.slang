#include "../tracing/trace.slang"

[numthreads(16, 1, 1)]
void main(uint3 idx: SV_DispatchThreadID)
{
    var id = idx.x;
    var max = p.ray_requests_ptr.numRays;

    if (id >= max)
        return;

    p.ray_results_ptr.numHits = max;

    var ray = p.ray_requests_ptr.rays[id];
    var result = traceScene(ray);
    p.ray_results_ptr.hits[id] = result;
}
