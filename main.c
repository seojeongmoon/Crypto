
//https://e2e.ti.com/support/tools/ccs/f/81/t/470171?Error-Memory-map-prevented-reading-0x202020B0-How-to-definitely-solve
//debug stops at 74 variables do not show

#define CRT 1
#define SIZE 1 // 0:=256Bit 1:=512Bit 2:=1024Bit

//#include <iostream>
#include <stdio.h>
#include "flint.h"
//#include "flint.c"
#include <limits.h>
#include <string.h>
#include <time.h>
#include "tm4c123gh6pm.h"

void squ_and_mul(clint* basis, clint* modulus, clint* erg, char* s);
clint* rsa(clint* base, clint* exp, clint* mod);
clint* rsa_crt(clint* base, clint* d, clint* p, clint* q);
void check_cipher(clint* c, char* c_ext);

unsigned short B = (unsigned short)65536;
unsigned short B2 = (unsigned short)2;
unsigned short B10 = (unsigned short)10;

// 1024 Bit
char ps_L[] = "11799940606765122157611277130045066959256062520371739560806392321926463280401194642712890850827847780551090208498805012553581203235294194860873223609015477";
char qs_L[] = "8957103306903606949408104239618127559192817675894527265487522259573418305677063719260073965144676083541693842217671539231745355197549057115339663240687569";
char ds_L[] = "56300298308155088980923609268016509861780477490973263650193072249759061565286809340268585664676608716992167748183091122265875131847771944006422174831430119093480602028582335571934965105491279674359750595983904975919602693879443303514332403980285859598567938751977320902254317213919790495804801654314650147713";
char es_L[] = "65537";
char ms_L[] = "9845036921172790349724138442481371302270379";
char cs_ext_L[] = "50571203937397858511647985320629927130537505837415149274830028958878116805341840267193085979130425395857900185830546862031315555840140494904011638299521958816249490480413153646508189217973175945409985001257778607199677484163934827815226595856763390792039693056398277961057506616842523912912758406648327660424";

// 512 Bit
char ps_M[] = "102468302676417273578692533748735034214350028164455124150937356262190356722939";
char qs_M[] = "85871286744099604471661168432181996245522858381258195361919919863335645951707";
char ds_M[] = "6588203625649244897983122339226530143073477588113406444528658124836291160727816210511494690029086008810320884067607895239984798354186811544403632594978553";
char es_M[] = "65537";
char ms_M[] = "9845036921172790349724138442481371302270379";
char cs_ext_M[] = "7301745151270516318623469090933370821244449555691537705291283029718157624242712959138659186919669630850613782800896611457929953588562350070003210682113769";

// 256 Bit
char ps_S[] = "250325044798691409684083071440443880431";
char qs_S[] = "229770138261794308748025352098251066273";
char ds_S[] = "24585041856782950592719418770956785764319358874029574779981960494202322200513";
char es_S[] = "65537";
char ms_S[] = "9845036921172790349724138442481371302270379";
char cs_ext_S[] = "38222019833090413175408435136713720884590177436526349868410614951574203042546";

char* ps[3] = { ps_S,ps_M,ps_L };
char* qs[3] = { qs_S,qs_M,qs_L };
char* ds[3] = { ds_S,ds_M,ds_L };
char* es[3] = { es_S,es_M,es_L };
char* ms[3] = { ms_S,ms_M,ms_L };
char* cs_ext[3] = { cs_ext_S,cs_ext_M,cs_ext_L };

int main()
{
	clock_t start;
	int diff_ms;

	char* ms_post;

	clint* p = create_l();
	clint* q = create_l();
	clint* n = create_l();
	clint* phi = create_l();
	clint* e = create_l();
	clint* d = create_l();
	clint* m = create_l();
	clint* c = create_l();
	clint* m_post = create_l();

	str2clint_l(p, ps[SIZE], B10);
	str2clint_l(q, qs[SIZE], B10);
	str2clint_l(d, ds[SIZE], B10);
	str2clint_l(e, es[SIZE], B10);
	str2clint_l(m, ms[SIZE], B10);

	mul_l(p, q, n);

	c = rsa(m, e, n);

	check_cipher(c, cs_ext[SIZE]);

	if (CRT) {
		start = clock();
		m_post = rsa_crt(c, d, p, q);
		diff_ms = (int)(clock() - start) * 1000 / CLOCKS_PER_SEC;
	}
	else {
		start = clock();
		m_post = rsa(c, d, n);
		diff_ms = (int)(clock() - start) * 1000 / CLOCKS_PER_SEC;
	}

	ms_post = xclint2str_l(m_post, B10, 0);

	if (equ_l(m,m_post)) {
		printf("\nInitial plain: %s \nPost plain:    %s\nEquality: %s\n\n", ms[SIZE], ms_post, "TRUE");
	}
	else {
		printf("\nInitial plain: %s \nPost plain:    %s\nEquality: %s\n\n", ms[SIZE], ms_post, "FALSE");
	}

	printf("Decrypt resp. verify time: %dms\n\n", diff_ms);

	return 0;
}

void squ_and_mul(clint* base, clint* mod, clint* res, char* e_bin) {
	SETONE_L(res);
	while (*e_bin) {					// Start with MSB
		msqr_l(res, res, mod);			// Mod to keep numbers small
		if (*e_bin == '1') {			// If bin. exp. digit == 1 mult. res by base
			mmul_l(res, base, res, mod);// Mod to keep numbers small
		}
		//printf("%s",xclint2str_l(res, B10, 0));
		e_bin++;
	}
}

clint* rsa(clint* base, clint* exp, clint* mod) {
	// encrypt crypt(plain, e, n)
	// decrypt crypt(cipher, d, n)
	// sign crypt(message, d, n)
	// verify crypt(signature, e, n)

	clint* res = create_l();
	char* e_bin = xclint2str_l(exp, 2, 0); // e_dez -> e_bin
	squ_and_mul(base, mod, res, e_bin);
	return res;
}

clint* rsa_crt(clint* base, clint* d, clint* p, clint* q) {
	// decrypt rsa_crt(cipher, d, p, q)
	// sign rsa_crt(message, d, p, q)

	clint* res = create_l();
	clint* p_dec = create_l();
	clint* q_dec = create_l();
	clint* dp = create_l();
	clint* dq = create_l();
	clint* bp = create_l();
	clint* bq = create_l();
	clint* u = create_l();
	clint* v = create_l();
	clint* n = create_l();
	clint* gcd = create_l();

	clint* t1 = create_l();
	clint* t2 = create_l();

	int sign_u, sign_v;
	int* ptr_sign_u = &sign_u;
	int* ptr_sign_v = &sign_v;

	mul_l(p, q, n);

	xgcd_l(p,q,gcd,u,ptr_sign_u,v,ptr_sign_v);

	if (sign_u < 0) sub_l(n, u, u);
	if (sign_v < 0) sub_l(n, v, v);

	// Following slide 72:
	sub_l(p, one_l, p_dec);
	sub_l(q, one_l, q_dec);

	mod_l(d, p_dec, dp);
	mod_l(d, q_dec, dq);

	bp = rsa(base, dp, p);
	bq = rsa(base, dq, q);

	mmul_l(p, bq, bq, n);
	mmul_l(u, bq, bq, n);
	mmul_l(q, bp, bp, n);
	mmul_l(v, bp, bp, n);

	madd_l(bq, bp, res, n);

	return res;
}

void check_cipher(clint* c, char* cs_ext) {
	clint* c_ext_l = create_l();
	str2clint_l(c_ext_l, cs_ext, B10);
	if (equ_l(c,c_ext_l)) printf("Check cipher: TRUE\n\n");
	else printf("Check cipher: FALSE\n\n");
}

// HOMEWORK:
/*char as[] = "10000000000000000000";
	char bs[] = "22222222222222222222";
	char* cs;

	clint* ap = create_l(); // Creates clint and returns pointer
	clint* bp = create_l();
	clint* cp = create_l();

	str2clint_l(ap, as, B10);
	str2clint_l(bp, bs, B10);

	mul_l(ap, bp, cp); // Apply plus minus as well

	cs = xclint2str_l(cp, B10, 0);

	printf(cs);*/
