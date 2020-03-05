#if __linux
#include <sys/syscall.h>
#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h> 
#endif

#include <unistd.h>

#include "asn/Condition.h"
#include "asn/Fulfillment.h"
#include "asn/OCTET_STRING.h"
#include "include/cJSON.h"
#include "cryptoconditions.h"
#include "internal.h"
#include "asn/OCTET_STRING.h"
#include "include/falcon/falcon.h"
#include "asn/Falcon512FingerprintContents.h"

struct CCType CC_Falcon512Type;


int cc_MakeFalcon512Signature(const unsigned char *msg32, const unsigned char *privateKey, unsigned char **signatureOut) {
  
    shake256_context rng;
    shake256_init_prng_from_system(&rng);

    void *pubkey, *sig;
    size_t pubkey_len, privkey_len, sig_len;
    size_t  tmpsd_len, tmpmp_len, tmpvv_len;
    uint8_t *tmpsd, *tmpmp, *tmpvv;

    unsigned logn = 9; // 9 is falcon 512
    pubkey_len = FALCON_PUBKEY_SIZE(logn); // not sure if we are using these lengths?
	privkey_len = FALCON_PRIVKEY_SIZE(logn);
	sig_len = FALCON_SIG_VARTIME_MAXSIZE(logn);
	
    tmpsd_len = FALCON_TMPSIZE_SIGNDYN(logn);
   	tmpmp_len = FALCON_TMPSIZE_MAKEPUB(logn);
    tmpvv_len = FALCON_TMPSIZE_VERIFY(logn);
       
    sig = malloc(sig_len);
    tmpsd = malloc(tmpsd_len);
	pubkey = malloc(pubkey_len);
    tmpmp = malloc(tmpmp_len);
    tmpvv = malloc(tmpvv_len);

    memset(sig, 0, sig_len);
    int error;
    error = falcon_make_public(pubkey, pubkey_len,
			privateKey, privkey_len, tmpmp, tmpmp_len);
		if (error != 0) {
			fprintf(stderr, "Falcon512 makepub failed: %d\n", error);
            return 0;
		}
    error = falcon_sign_dyn(&rng, sig, &sig_len,
			privateKey, privkey_len,
			(const void*)msg32, sizeof(msg32), 0, tmpsd, tmpsd_len);
     if (error != 0) {
			fprintf(stderr, "Falcon512 keygen failed: %d\n", error);
            return 0;
		}

    error = falcon_verify(sig, sig_len,
			pubkey, pubkey_len, (const void*)msg32, sizeof(msg32), tmpvv, tmpvv_len);
		if (error != 0) {
			fprintf(stderr, "Falcon512 verify failed: %d\n", error);
            return 0;
		}



    *signatureOut = calloc(1, FALCON_SIG_CT_SIZE(logn)); //not sure of length of falcon signture out are we using duynamic?

    memcpy(signatureOut,sig,sig_len);

    free(sig);
    free(tmpsd);
	free(pubkey);
    free(tmpmp);
    free(tmpvv);
    
    return 1;
}

int cc_MakeFalcon512KeyPair(unsigned char *privateKey, unsigned char *publicKey)
{
    shake256_context rng; uint8_t *tmpkg;
    unsigned logn = 9; // 9 is falcon 512
    shake256_init_prng_from_system(&rng);
    tmpkg = malloc(FALCON_TMPSIZE_KEYGEN(logn));

    //privateKey and publicKey need memory allocated to them before falcone_keygen_make can run
    int success = falcon_keygen_make(&rng, logn, privateKey, FALCON_PRIVKEY_SIZE(logn),
			publicKey, FALCON_PUBKEY_SIZE(logn), tmpkg, FALCON_TMPSIZE_KEYGEN(logn));

    if (success != 0) {
			fprintf(stderr, "keygen failed: %d\n", success);
			return 0;
		}

    return 1;    
}

int cc_VerifyFalcon512Key(const unsigned char *msg32, const unsigned char *publicKey, unsigned char *signature){  
    
    shake256_context rng;
    shake256_init_prng_from_system(&rng);

    size_t pubkey_len, sig_len;
    size_t tmpvv_len;
    uint8_t *tmpvv;

    unsigned logn = 9; // 9 is falcon 512
    pubkey_len = FALCON_PUBKEY_SIZE(logn); // not sure if we are using these lengths?

	sig_len = FALCON_SIG_VARTIME_MAXSIZE(logn);

    tmpvv_len = FALCON_TMPSIZE_VERIFY(logn);

    tmpvv = malloc(tmpvv_len);

   int  error = falcon_verify(signature, sig_len,
			publicKey, pubkey_len, (const void*)msg32, sizeof(msg32), tmpvv, tmpvv_len);
		if (error != 0) {
			fprintf(stderr, "Falcon512 verify failed: %d\n", error);
            return 0;
		}

    return 1;
}

static unsigned char *falcon512Fingerprint(const CC *cond) {
    Falcon512FingerprintContents_t *fp = calloc(1, sizeof(Falcon512FingerprintContents_t));
    OCTET_STRING_fromBuf(&fp->publicKey, cond->publicKey, FALCON_PUBKEY_SIZE(9));
    return hashFingerprintContents(&asn_DEF_Falcon512FingerprintContents, fp);
}

static unsigned long falcon512Cost(const CC *cond) {
    return 131072;
}


//static CC *cc_falcon512Condition(const unsigned char *publicKey, const unsigned char *signature) {
  
  //  return 0;
//}


static CC *falcon512FromJSON(const cJSON *params, char *err) {
  //  CC *cond = 0;
  //  unsigned char *pk = 0, *sig = 0;
 //   size_t pkSize, sigSize;

    return 0;
}


static void falcon512ToJSON(const CC *cond, cJSON *params) {

}


static CC *falcon512FromFulfillment(const Fulfillment_t *ffill) {
    return 0;
}


static Fulfillment_t *falcon512ToFulfillment(const CC *cond) {
    if (!cond->signature) {
        return NULL;
    }

    return 0;
}


static CC *falcon512FromPartialFulfillment(const Fulfillment_t *ffill) {
    return 0;
}


static Fulfillment_t *falcon512ToPartialFulfillment(const CC *cond) {

    
    return 0;
}


int falcon512IsFulfilled(const CC *cond) {
    return cond->signature != 0;
}


static void falcon512Free(CC *cond) {
    free(cond->publicKey);
    if (cond->signature) {
        free(cond->signature);
    }
}


static uint32_t falcon512Subtypes(const CC *cond) {
    return 0;
}

struct CCType CC_Falcon512Type = { 5, "falcon512-sha-256", Condition_PR_falcon512, 0, &falcon512Fingerprint, &falcon512Cost, &falcon512Subtypes, &falcon512FromJSON, &falcon512ToJSON, &falcon512FromFulfillment, &falcon512ToFulfillment, &falcon512FromPartialFulfillment, &falcon512ToPartialFulfillment, &falcon512IsFulfilled, &falcon512Free };