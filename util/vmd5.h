#ifndef VMD5_H
#define VMD5_H

#ifdef __cplusplus
extern "C"
{
#endif

#define md5byte unsigned char

struct MD5Context
{
    unsigned int buf[4];
    unsigned int bytes[2];
    unsigned int in[16];
};

void MD5_init(struct MD5Context *context);
void MD5_update(struct MD5Context *context, md5byte const *buf, unsigned len);
void MD5_final(unsigned char digest[16], struct MD5Context *context);
void MD5_transform(unsigned int buf[4], unsigned int const in[16]);

#ifdef __cplusplus
}
#endif

#endif // VMD5_H
