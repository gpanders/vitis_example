#include "axpy.h"

#ifdef __cplusplus
extern "C" {
#endif

void axpy(struct bus const *a, struct bus const *x, struct bus const *y, struct bus *out, const int len)
{
#pragma HLS DATA_PACK variable=a
#pragma HLS DATA_PACK variable=x
#pragma HLS DATA_PACK variable=y
#pragma HLS DATA_PACK variable=out
#pragma HLS INTERFACE m_axi port=a offset=slave max_read_burst_length=256 bundle=gmem0
#pragma HLS INTERFACE m_axi port=x offset=slave max_read_burst_length=256 bundle=gmem0
#pragma HLS INTERFACE m_axi port=y offset=slave max_read_burst_length=256 bundle=gmem0
#pragma HLS INTERFACE m_axi port=out offset=slave max_write_burst_length=256 bundle=gmem1
#pragma HLS INTERFACE s_axilite port=a bundle=control
#pragma HLS INTERFACE s_axilite port=x bundle=control
#pragma HLS INTERFACE s_axilite port=y bundle=control
#pragma HLS INTERFACE s_axilite port=out bundle=control
#pragma HLS INTERFACE s_axilite port=len bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

	for (int i = 0; i < len / PACKSIZE; i++) {
#pragma HLS PIPELINE II=1 rewind
		for (int j = 0; j < PACKSIZE; j++) {
			out[i].arr[j] = a[i].arr[j]*x[i].arr[j] + y[i].arr[j];
		}
	}
}

#ifdef __cplusplus
}
#endif
