/* Simple S/MIME decryption example */
#include <openssl/pem.h>
#include <openssl/cms.h>
#include <openssl/err.h>

int main(int argc, char **argv)
	{
	BIO *in = NULL, *out = NULL, *tbio = NULL;
	X509 *rcert = NULL;
	EVP_PKEY *rkey = NULL;
	CMS_ContentInfo *cms = NULL;
	int ret = 1;

	OpenSSL_add_all_algorithms();
	ERR_load_crypto_strings();

	/* Read in recipient certificate and private key */
	tbio = BIO_new_file("signer.pem", "r");

	if (!tbio)
		goto err;

	rcert = PEM_read_bio_X509(tbio, NULL, 0, NULL);

	BIO_reset(tbio);

	rkey = PEM_read_bio_PrivateKey(tbio, NULL, 0, NULL);

	if (!rcert || !rkey)
		goto err;

	/* Open S/MIME message to decrypt */

	in = BIO_new_file("smencr.txt", "r");

	if (!in)
		goto err;

	/* Parse message */
	cms = SMIME_read_CMS(in, NULL);

	if (!cms)
		goto err;

	out = BIO_new_file("decout.txt", "w");
	if (!out)
		goto err;

	/* Decrypt S/MIME message */
	if (!CMS_decrypt(cms, rkey, rcert, NULL, out, 0))
		goto err;

	ret = 0;

	err:

	if (ret)
		{
		fprintf(stderr, "Error Decrypting Data\n");
		ERR_print_errors_fp(stderr);
		}

	if (cms)
		CMS_ContentInfo_free(cms);
	if (rcert)
		X509_free(rcert);
	if (rkey)
		EVP_PKEY_free(rkey);

	if (in)
		BIO_free(in);
	if (out)
		BIO_free(out);
	if (tbio)
		BIO_free(tbio);

	return ret;

	}
