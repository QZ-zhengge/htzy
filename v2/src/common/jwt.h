// JWT utility - Pure C++ implementation (NO OpenSSL dependency)
// HS256 using minimal HMAC-SHA256 in pure C++
#pragma once

#include <string>
#include <vector>
#include <ctime>
#include <cstring>

namespace jwt {

// ==================== SHA-256 (RFC 6234) ====================
static const uint32_t K[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

#define RR(x,n) (((x)>>(n))|((x)<<(32-(n))))
#define CH(x,y,z) (((x)&(y))^((~(x))&(z)))
#define MAJ(x,y,z) (((x)&(y))^((x)&(z))^((y)&(z)))
#define EP0(x) (RR(x,2)^RR(x,13)^RR(x,22))
#define EP1(x) (RR(x,6)^RR(x,11)^RR(x,25))
#define SIG0(x) (RR(x,7)^RR(x,18)^((x)>>3))
#define SIG1(x) (RR(x,17)^RR(x,19)^((x)>>10))

static void sha256_transform(uint32_t state[8], const unsigned char block[64]) {
    uint32_t W[64], a, b, c, d, e, f, g, h, t1, t2;
    for (int i = 0; i < 16; ++i)
        W[i] = ((uint32_t)block[i*4]<<24)|((uint32_t)block[i*4+1]<<16)|((uint32_t)block[i*4+2]<<8)|block[i*4+3];
    for (int i = 16; i < 64; ++i)
        W[i] = SIG1(W[i-2]) + W[i-7] + SIG0(W[i-15]) + W[i-16];
    a=state[0]; b=state[1]; c=state[2]; d=state[3];
    e=state[4]; f=state[5]; g=state[6]; h=state[7];
    for (int i = 0; i < 64; ++i) {
        t1 = h + EP1(e) + CH(e,f,g) + K[i] + W[i];
        t2 = EP0(a) + MAJ(a,b,c);
        h=g; g=f; f=e; e=d+t1; d=c; c=b; b=a; a=t1+t2;
    }
    state[0]+=a; state[1]+=b; state[2]+=c; state[3]+=d;
    state[4]+=e; state[5]+=f; state[6]+=g; state[7]+=h;
}

static std::string sha256(const std::string& data) {
    uint32_t state[8] = {0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
    unsigned char buf[64];
    uint64_t bitlen = data.size() * 8;
    size_t len = data.size();
    const unsigned char* msg = reinterpret_cast<const unsigned char*>(data.data());
    for (size_t i = 0; i + 64 <= len; i += 64)
        sha256_transform(state, msg + i);
    size_t remaining = len % 64;
    memset(buf, 0, 64);
    memcpy(buf, msg + len - remaining, remaining);
    buf[remaining] = 0x80;
    if (remaining >= 56) {
        sha256_transform(state, buf);
        memset(buf, 0, 64);
    }
    for (int i = 0; i < 8; ++i)
        buf[56+i] = static_cast<unsigned char>(bitlen >> (56 - i*8));
    sha256_transform(state, buf);
    std::string out(32, '\0');
    for (int i = 0; i < 8; ++i) {
        out[i*4]   = (state[i]>>24)&0xff;
        out[i*4+1] = (state[i]>>16)&0xff;
        out[i*4+2] = (state[i]>>8)&0xff;
        out[i*4+3] = state[i]&0xff;
    }
    return out;
}

// HMAC-SHA256
static std::string hmac_sha256(const std::string& key, const std::string& data) {
    const size_t BLOCK = 64;
    std::string k = key.size() > BLOCK ? sha256(key) : key;
    k.resize(BLOCK, '\0');
    std::string ipad(BLOCK, 0x36), opad(BLOCK, 0x5c);
    for (size_t i = 0; i < BLOCK; ++i) {
        ipad[i] ^= k[i];
        opad[i] ^= k[i];
    }
    return sha256(opad + sha256(ipad + data));
}

// Base64url
static std::string base64url_encode(const std::string& input) {
    static const char T[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    std::string out;
    out.reserve(((input.size()+2)/3)*4);
    unsigned int v=0; int b=-6;
    for (unsigned char c : input) { v=(v<<8)+c; b+=8; while(b>=0){out+=T[(v>>b)&0x3F];b-=6;} }
    if(b>-6) out+=T[((v<<8)>>(b+8))&0x3F];
    return out;
}

static std::string base64url_decode(const std::string& input) {
    static const int T[256] = {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,63,
        52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
        15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
        -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
        41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
    };
    std::string out; unsigned int v=0; int b=-8;
    for(unsigned char c:input){int d=T[c]; if(d<0)continue; v=(v<<6)+d; b+=6; if(b>=0){out+=(char)((v>>b)&0xff);b-=8;}}
    return out;
}

// JWT sign
static std::string sign(const std::string& payload, const std::string& secret) {
    std::string header = "{\"alg\":\"HS256\",\"typ\":\"JWT\"}";
    std::string eh = base64url_encode(header);
    std::string ep = base64url_encode(payload);
    std::string sig = base64url_encode(hmac_sha256(secret, eh+"."+ep));
    return eh+"."+ep+"."+sig;
}

// Create token
static std::string create(const std::string& user_id, const std::string& username,
                          const std::string& secret, int expiry_seconds = 86400) {
    std::time_t now = std::time(nullptr);
    std::string payload = "{\"sub\":\""+user_id+"\",\"name\":\""+username+"\"," +
        "\"iat\":" + std::to_string(now) + ",\"exp\":" + std::to_string(now+expiry_seconds) + "}";
    return sign(payload, secret);
}

// Verify token -> user_id (empty if invalid)
static std::string verify(const std::string& token, const std::string& secret) {
    auto d1 = token.find('.'); if(d1==std::string::npos) return "";
    auto d2 = token.find('.', d1+1); if(d2==std::string::npos) return "";
    std::string eh=token.substr(0,d1), ep=token.substr(d1+1,d2-d1-1), es=token.substr(d2+1);
    if(es!=base64url_encode(hmac_sha256(secret, eh+"."+ep))) return "";
    std::string payload = base64url_decode(ep);
    auto p = payload.find("\"exp\":"); if(p==std::string::npos) return "";
    auto s=p+6, e=payload.find_first_of(",}",s);
    long exp = std::stol(payload.substr(s,e-s));
    if(std::time(nullptr)>exp) return "";
    auto q = payload.find("\"sub\":\""); if(q==std::string::npos) return "";
    auto qs=q+7, qe=payload.find('"',qs);
    return payload.substr(qs,qe-qs);
}

#undef RR
#undef CH
#undef MAJ
#undef EP0
#undef EP1
#undef SIG0
#undef SIG1

} // namespace jwt
