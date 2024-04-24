#include "CacheSim.h"
#include <unistd.h>
#include <stdio.h>

/* Posa aqui les teves estructures de dades globals
 * per mantenir la informacio necesaria de la cache
 * */
struct linea{
	int TAG;
	int bytes[32];
};

struct linea cache [128];
unsigned int hit_g;
unsigned int miss_g;


/* La rutina init_cache es cridada pel programa principal per
 * inicialitzar la cache.
 * La cache es inicialitzada al comen�ar cada un dels tests.
 * */
void init_cache ()
{
	/* Escriu aqui el teu codi */
	hit_g = 0;
	miss_g = 0;
	for (int i = 0; i < 128; ++i) {
		cache[i].TAG = -1;
		for (int j = 0; j < 32;++j) cache[i].bytes[j] = -1;
	}
}

/* La rutina reference es cridada per cada referencia a simular */ 
void reference (unsigned int address, unsigned int LE)
{
	//LE = 0 lectura, LE = 1 escritura
	unsigned int byte;
	unsigned int bloque_m; 
	unsigned int linea_mc;
	unsigned int tag;
	unsigned int miss;
	unsigned int lec_mp;
	unsigned int mida_lec_mp;
	unsigned int esc_mp;
	unsigned int mida_esc_mp;
	unsigned int replacement;
	unsigned int tag_out;

	/* Escriu aqui el teu codi */
	byte = address%32;
	bloque_m = address/32;
	linea_mc = bloque_m%128;
	tag = bloque_m/128;

	if (cache[linea_mc].TAG == tag) {
		miss = 0;
		++hit_g;
		lec_mp = 0;
		mida_lec_mp = 0;
		if (LE == 1) {
			esc_mp = 1;
			mida_esc_mp = 1;
		}
		else {
			esc_mp = 0;
			mida_esc_mp = 0;
		}
		replacement = 0;
	}
	else {
		miss = 1;
		++miss_g;
		if (LE == 0) {
			lec_mp = 1;
			mida_lec_mp = 32;
			esc_mp = 0;
			mida_esc_mp = 0;
			if (cache[linea_mc].TAG == -1) {
				replacement = 0;
				cache[linea_mc].TAG = tag;
			}
			else {
				replacement = 1;
				tag_out = cache[linea_mc].TAG;
				cache[linea_mc].TAG = tag;
			}
		}
		else {
			lec_mp = 0;
			mida_lec_mp = 0;
			esc_mp = 1;
			mida_esc_mp = 1;
			replacement = 0;
		}
	}
	/* La funcio test_and_print escriu el resultat de la teva simulacio
	 * per pantalla (si s'escau) i comproba si hi ha algun error
	 * per la referencia actual
	 * */
	test_and_print (address, LE, byte, bloque_m, linea_mc, tag,
			miss, lec_mp, mida_lec_mp, esc_mp, mida_esc_mp,
			replacement, tag_out);
}

/* La rutina final es cridada al final de la simulacio */ 
void final ()
{
 	/* Escriu aqui el teu codi */ 
	int ret;
	char buff[64];
	ret = sprintf(buff, "Hits: %d\nMiss: %d\n", hit_g, miss_g);
	write(1, buff, ret);
}
