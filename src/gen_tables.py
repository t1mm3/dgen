#!/bin/env python
def gen_int2_str_lut(ctype, dmin, dmax):

	print "static constexpr Int2StrLUT {t}_int2str_lut[] = {{".format(t=ctype)

	num = 0
	for i in range(dmin, dmax+1):
		print """{{ "{s}", sizeof("{s}")-1 }}, """.format(s=i)
		num = num + 1
	print "};"

	print "static constexpr int32_t {t}_int2str_lut_len = {len};".format(t=ctype, len=num)


print """
// Auto generated
#ifndef H_GEN_GEN_TABLES
#define H_GEN_GEN_TABLES

struct Int2StrLUT {{
	const char* s;
	size_t l;
}};
""".format()

gen_int2_str_lut("int8_t", -128, 127)
gen_int2_str_lut("uint8_t", 0, 255)
gen_int2_str_lut("int16_t", -32768, 32767)
gen_int2_str_lut("uint16_t", 0, 65535)

print """
#endif
"""