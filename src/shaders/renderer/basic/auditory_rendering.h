#pragma once

//auditory tracing functions
rtBuffer<float2>    sound_trace; //stores variables for ray for sound (eqals prd data???)
rtBuffer<float>    sound_touch; //when the index prim should be highlighted
rtDeclareVariable(float, TimeSound, , ); //global for tracing sound


//scanning variables
rtDeclareVariable(float, line_scan, , );
rtDeclareVariable(int, scan_all, , );


__device__ bool fillsCondition(float y)
{

    if (scan_all == 0)
    {
        //printf("scanning only one line %f\n", line_scan);
        if (abs(y - line_scan) <= 0.01f)
            return true;
        else return false;

    }
    else
    {
        if (abs(y) <= 1.0f) // one of our lines
        {
            //float x, y, n;
            float yy = abs(y*10.0f);
            //y = fmodf(x, n);
            float valY = yy - floorf(yy);
            //val = val/x;
            if (valY < 0.01f)
            {
                return true;
            }
            else return false;
        }
        else return false;
    }
}



__device__ bool isSoundRay(int&num, optix::float2 d, optix::float2 launch_index, optix::float2 screen)
{

    if (fillsCondition(d.y))
    {

        int numY = floorf(d.y*10.0f);
        if (d.y < 0)  numY += 10;

        if (abs(d.x) <= 1.0f) // one of our lines
        {
            //float x, y, n;
            float xx = abs(d.x*10.0f);
            //y = fmodf(x, n);
            float val = xx - floorf(xx);
            //val = val/x;
            if (val < 0.02f)
            {
                //if (numY > 0)
                //printf("%d\n", numY);
                //size_t2 screen = output_buffer.size();
                //float2 subpixel_jitter = make_float2(0.0f, 0.0f);
                //float2 d = (make_float2(launch_index) + subpixel_jitter) / make_float2(screen) * 2.f - 1.f;
                //get back to launch_index.x and use it as num
                num = 10 * numY + launch_index.x + 1; //consider launch_index.x<MAXDIM
                return true;
            }
            else return false;

        }
        else return false;
    }

return false;
}


//TODO: 
//implement highlightment with several materials (don't know)

//or

//prd.compute_sound =true ///set that for this ray sound should be computed
//