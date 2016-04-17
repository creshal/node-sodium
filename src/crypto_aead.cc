/*
 * Node Native Module for Lib Sodium
 *
 * @Author Pedro Paixao
 * @email paixaop at gmail dot com
 * @License MIT
 */
#include "node_sodium.h"
#include "crypto_aead.h"

/***
 * Authenticated Encryption with Additional Data:
 *
 * Encrypts a message with a key and a nonce to keep it confidential Computes
 * an authentication tag. This tag is used to make sure that the message, as
 * well as optional, non-confidential (non-encrypted) data, haven't been
 * tampered with.
 *
 * A typical use case for additional data is to store protocol-specific metadata
 * about the message, such as its length and encoding.
 *
 * Supported Algorithms:
 *  * AES-GCM 256: API names use `aes256gcm`
 *  * ChaCha20-Poly1305: API names use `chacha20poly1305`
 *  * ChaCha20-Poly1305-IETF: API names use `chacha20poly1305-ietf`
 *
 * ### Modes
 *
 * #### Combined
 *  In combined mode, the authentication tag and the encrypted
 *  message are stored together. Functions return a buffer that includes the
 *  cipher text and authentication tag.
 *  Encrypt/Decrypt functions return a buffer with length equal to
 *  `message_length + crypto_aead_*_ABYTES` bytes.
 *
 * #### Detached
 *  In detached mode, the authentication tag and the encrypted
 *  message in different buffers. Detached function variants are named with the
 *  `_detached` sufix. Encrypt functions return:
 *
 *    { cipherText: <buffer>, mac: <buffer> }
 *
 * ~ cipherText (Buffer): encrypted message
 * ~ mac (Buffer): authentication tag (`crypto_aead_*_ABYTES` long)
 *
 * ### Constants
 * Replace `ALGORITHM` with one of the supported algorithms (`aes256gcm`,
 * `chacha20poly1305`, or `chacha20poly1305-ietf`)
 *
 * ~ crypto_aead_ALGORITHM_ABYTES: length of the authentication tag buffer
 * ~ crypto_aead_ALGORITHM_KEYBYTES: length of secret key
 * ~ crypto_aead_ALGORITHM_NPUBBYTES: lenght of public nonce
 * ~ crypto_aead_ALGORITHM_NSECBYTES: length of secret nonce. Not used
 */

/**
 * Crypto AEAD AES-GCM 256:
 *
 * The current implementation of this construction is hardware-accelerated and
 * requires the Intel SSSE3 extensions, as well as the aesni and pclmul
 * instructions.
 *
 * Intel Westmere processors (introduced in 2010) and newer meet the requirements.
 *
 * There are no plans to support non hardware-accelerated implementations of
 * [AES-GCM](https://en.wikipedia.org/wiki/Galois/Counter_Mode).
 * If portability is a concern, use ChaCha20-Poly1305 instead.
 */

/** Low Level API: */

/**
 * Precompute API:
 *
 * Speeding up the encryption/decryption process.
 * The precompute API breaks down the encrypt/decrypt functions in two stages -
 * "Before" and "After". The "before" stage is called once, everytime your
 * application changes encryption keys. The "after" stage is called for every
 * message you need to encrypt/decrypt.
 * The functions `*_beforenm` implement the "before" stage, and the `*_afternm`
 * the "after" stage. They offer the same functionality as their non-precompute
 * counterparts but take a "state" object generated by the `*_beforenm` instead
 * of an encryption key.
 *
 * **Sample**:
 *
 *     var sodium = require('sodium').api;
 *
 *     // Generate a random key
 *     var key = new Buffer(crypto_aead_aes256gcm_KEYBYTES);
 *     sodium.randombytes_buf(key);
 *
 *     // Generate random nonce
 *     var nonce = new Buffer(crypto_aead_aes256gcm_KEYBYTES);
 *     sodium.randombytes_buf(nonce);
 *
 *     // Precompute and generate the state
 *     var state = sodium.crypto_aead_aes256gcm_beforenm(key);
 *
 *     var message = new Buffer("this is a plain text message");
 *     var additionalData = new Buffer("metadata");
 *
 *     // Encrypt Data
 *     var cipherText = sodium.crypto_aead_aes256gcm_encrypt_afternm(
 *        message, additionalData, nonce, state);
 *
 *     // Get the plain text, i.e., original message back
 *     var plainText = sodium.crypto_aead_aes256gcm_decrypt_afternm(
 *        cipherText, additionalData, nonce, state);
 */

/**
 * crypto_aead_aes256gcm_is_available:
 *
 * Check hardware support for AES 256 GCM
 *
 * **Returns**:
 *
 * ~ true: if hardware supports AES 256 GCM
 *
 * **Sample**:
 *
 *   if( sodium.crypto_aead_aes256gcm_is_available() ) {
 *     // You can use the crypto_aead_aes256gcm_*()
 *   }
 *   else {
 *     // Use crypto_aead_chacha20poly1305_*()
 *   }
 *
 */
NAN_METHOD(bind_crypto_aead_aes256gcm_is_available) {
    Nan::EscapableHandleScope scope;

    if( crypto_aead_aes256gcm_is_available() == 1 ) {
        return info.GetReturnValue().Set(Nan::True());
    }

    return info.GetReturnValue().Set(Nan::False());
}


/**
 * crypto_aead_aes256gcm_beforenm:
 * Precompute AES key expansion.
 *
 * Applications that encrypt several messages using the same key can gain a
 * little speed by expanding the AES key only once, via the precalculation interface
 * Initializes a context ctx by expanding the key k and always returns 0.
 *
 *   var ctx = sodium.crypto_aead_aes256gcm_beforenm(key);
 *
 * ~ key (Buffer):  AES 256 GCM Key buffer with crypto_aead_aes256gcm_KEYBYTES in length
 *
 * **Sample**:
 *
 *     // Generate a random key
 *     var key = new Buffer(crypto_aead_aes256gcm_KEYBYTES);
 *     sodium.randombytes_buf(key);
 *     var state = sodium.crypto_aead_aes256gcm_beforenm(key);
 */
NAN_METHOD(bind_crypto_aead_aes256gcm_beforenm) {
    Nan::EscapableHandleScope scope;

    ARGS(1,"arguments key must be a buffer");
    ARG_TO_UCHAR_BUFFER_LEN(key, crypto_aead_aes256gcm_KEYBYTES);

    NEW_BUFFER_AND_PTR(ctxt, crypto_aead_aes256gcm_statebytes());

    if (crypto_aead_aes256gcm_beforenm((crypto_aead_aes256gcm_state*)ctxt_ptr, key) == 0) {
        return info.GetReturnValue().Set(ctxt);
    }

    return info.GetReturnValue().Set(Nan::Undefined());
}

/**
 * crypto_aead_aes256gcm_encrypt_afternm:
 * Encrypt data in Combined Mode
 *
 *    var c = sodium.crypto_aead_aes256gcm_encrypt_afternm(
 *              message,
 *              additionalData,
 *              nonce,
 *              ctx);
 *
 * ~ message (Buffer): plain text buffer
 * ~ additionalData (Buffer): non-confidential data to add to the cipher text. Can be `null`
 * ~ nonce (Buffer): a nonce with `sodium.crypto_aead_aes256gcm_NPUBBYTES` in length
 * ~ ctx (Buffer): state computed by `crypto_aead_aes256gcm_beforenm()`
 *
 * **Returns**:
 *
 * ~ cipherText (Buffer): The encrypted message, as well as a tag authenticating
 *   both the confidential message `message` and non-confidential data `additionalData`
 * ~ undefined: if `message` fails to encrypt
 *
 */
NAN_METHOD(bind_crypto_aead_aes256gcm_encrypt_afternm) {
    Nan::EscapableHandleScope scope;

    ARGS(4,"arguments message, additional data, nonce, and key must be buffers");
    ARG_TO_UCHAR_BUFFER(m);
    ARG_TO_UCHAR_BUFFER_OR_NULL(ad);
    ARG_TO_UCHAR_BUFFER_LEN(npub, crypto_aead_aes256gcm_NPUBBYTES);
    ARG_TO_VOID_BUFFER_LEN(ctx, crypto_aead_aes256gcm_statebytes());

    NEW_BUFFER_AND_PTR(c, crypto_aead_aes256gcm_ABYTES + m_size);
    sodium_memzero(c_ptr, crypto_aead_aes256gcm_ABYTES + m_size);
    unsigned long long clen;

    if( crypto_aead_aes256gcm_encrypt_afternm (c_ptr, &clen, m, m_size, ad, ad_size, NULL, npub, (crypto_aead_aes256gcm_state*)ctx) == 0 ) {
        return info.GetReturnValue().Set(c);
    }
    return info.GetReturnValue().Set(Nan::Undefined());
}

/**
 * crypto_aead_aes256gcm_decrypt_afternm:
 * Decrypt data in Combined Mode
 *
 *    var c = sodium.crypto_aead_aes256gcm_decrypt_afternm(
 *              cipherText,
 *              additionalData,
 *              nonce,
 *              ctx);
 *
 * ~ cipherText (Buffer): cipher text buffer, encrypted by crypto_aead_aes256gcm_encrypt_afternm()
 * ~ additionalData (Buffer): non-confidential data to add to the cipher text. Can be `null`
 * ~ nonce (Buffer): a nonce with `sodium.crypto_aead_aes256gcm_NPUBBYTES` in length
 * ~ ctx (Buffer): state computed by `crypto_aead_aes256gcm_beforenm()`
 *
 * **Returns**:
 *
 * ~ message (Buffer): plain text message
 * ~ undefined: if `cipherText` is not valid
 *
 */
NAN_METHOD(bind_crypto_aead_aes256gcm_decrypt_afternm) {
    Nan::EscapableHandleScope scope;

    ARGS(4,"arguments chiper text, additional data, nonce, and key must be buffers");
    ARG_TO_UCHAR_BUFFER(c);
    if( c_size < crypto_aead_aes256gcm_ABYTES ) {
        std::ostringstream oss;
        oss << "argument cipher text must be at least " <<  crypto_aead_aes256gcm_ABYTES << " bytes long" ;
        return Nan::ThrowError(oss.str().c_str());
    }
    ARG_TO_UCHAR_BUFFER_OR_NULL(ad);
    ARG_TO_UCHAR_BUFFER_LEN(npub, crypto_aead_aes256gcm_NPUBBYTES);
    ARG_TO_VOID_BUFFER_LEN(ctx, crypto_aead_aes256gcm_statebytes());

    NEW_BUFFER_AND_PTR(m, c_size - crypto_aead_aes256gcm_ABYTES);
    unsigned long long mlen;

    if( crypto_aead_aes256gcm_decrypt_afternm (m_ptr, &mlen, NULL, c, c_size, ad, ad_size, npub, (crypto_aead_aes256gcm_state*)ctx) == 0 ) {
        return info.GetReturnValue().Set(m);
    }

    return info.GetReturnValue().Set(Nan::Undefined());
}

/**
 * crypto_aead_aes256gcm_encrypt_detached_afternm:
 * Encrypt data in Detached Mode
 *
 *    var c = sodium.crypto_aead_aes256gcm_encrypt_detached_afternm(
 *              message,
 *              additionalData,
 *              nonce,
 *              ctx);
 *
 * ~ message (Buffer): plain text buffer
 * ~ additionalData (Buffer): non-confidential data to add to the cipher text.
 *   Can be `null`
 * ~ nonce (Buffer): a nonce with `sodium.crypto_aead_aes256gcm_NPUBBYTES` in
 *   length
 * ~ ctx (Buffer): state computed by `crypto_aead_aes256gcm_beforenm()`
 *
 * **Returns**:
 *
 * ~ object: `cipherText` buffer with ciphered text, and `mac` buffer with the
 *   authentication tag
 *
 *    { cipherText: <buffer>, mac: <buffer> }
 *
 * ~ undefined: if `message` fails to encrypt
 *
 * **Sample**:
 *
 *     var sodium = require('sodium').api;
 *
 *     // Generate a random key
 *     var key = new Buffer(crypto_aead_aes256gcm_KEYBYTES);
 *     sodium.randombytes_buf(key);
 *
 *     // Generate random nonce
 *     var nonce = new Buffer(crypto_aead_aes256gcm_KEYBYTES);
 *     sodium.randombytes_buf(nonce);
 *
 *     // Precompute and generate the state
 *     var state = sodium.crypto_aead_aes256gcm_beforenm(key);
 *
 *     var message = new Buffer("this is a plain text message");
 *     var additionalData = new Buffer("metadata");
 *
 *     // Encrypt Data
 *     var c = sodium.crypto_aead_aes256gcm_encrypt_detached_afternm(
 *        message, additionalData, nonce, state);
 *
 *     // Get the plain text, i.e., original message back
 *     var plainText = sodium.crypto_aead_aes256gcm_decrypt_detached_afternm(
 *        c.cipherText, c.mac, nonce, state);
 */
NAN_METHOD(bind_crypto_aead_aes256gcm_encrypt_detached_afternm) {
    Nan::EscapableHandleScope scope;

    ARGS(4,"arguments message, additional data, nonce, and key must be buffers");
    ARG_TO_UCHAR_BUFFER(m);
    ARG_TO_UCHAR_BUFFER_OR_NULL(ad);
    ARG_TO_UCHAR_BUFFER_LEN(npub, crypto_aead_aes256gcm_NPUBBYTES);
    ARG_TO_VOID_BUFFER_LEN(ctx, crypto_aead_aes256gcm_statebytes());

    NEW_BUFFER_AND_PTR(c, m_size);
    NEW_BUFFER_AND_PTR(mac, crypto_aead_aes256gcm_ABYTES);

    if( crypto_aead_aes256gcm_encrypt_detached_afternm(c_ptr, mac_ptr, NULL, m, m_size, ad, ad_size, NULL, npub, (crypto_aead_aes256gcm_state*)ctx) == 0 ) {
        Local<Object> result = Nan::New<Object>();
        result->ForceSet(Nan::New<String>("cipherText").ToLocalChecked(), c, DontDelete);
        result->ForceSet(Nan::New<String>("mac").ToLocalChecked(), mac, DontDelete);
        return info.GetReturnValue().Set(result);
    }

    return info.GetReturnValue().Set(Nan::Undefined());
}

/**
 * crypto_aead_aes256gcm_decrypt_detached_afternm:
 * Encrypt data in Detached Mode
 *
 *    var c = sodium.crypto_aead_aes256gcm_decrypt_detached_afternm(
 *              cipherText,
 *              mac,
 *              nonce,
 *              ctx);
 *
 * ~ cipherText (Buffer): cipher text buffer
 * ~ mac (Buffer): authentication tag. Can be null
 * ~ nonce (Buffer): a nonce with `sodium.crypto_aead_aes256gcm_NPUBBYTES` in
 *   length
 * ~ ctx (Buffer): state computed by `crypto_aead_aes256gcm_beforenm()`
 *
 * **Returns**:
 *
 * ~ message (Buffer): plain text message
 * ~ undefined: if `message` fails to encrypt
 *
 * See: [crypto_aead_aes256gcm_encrypt_detached_afternm](#crypto_aead_aes256gcm_encrypt_detached_afternm)
 *
 */
NAN_METHOD(bind_crypto_aead_aes256gcm_decrypt_detached_afternm) {
    Nan::EscapableHandleScope scope;

    ARGS(4,"arguments cipher message, mac, additional data, nsec, nonce, and key must be buffers");
    ARG_TO_UCHAR_BUFFER(c);
    ARG_TO_UCHAR_BUFFER_LEN(mac, crypto_aead_aes256gcm_ABYTES);
    ARG_TO_UCHAR_BUFFER_OR_NULL(ad);
    ARG_TO_UCHAR_BUFFER_LEN(npub, crypto_aead_aes256gcm_NPUBBYTES);
    ARG_TO_VOID_BUFFER_LEN(ctx, crypto_aead_aes256gcm_statebytes());

    NEW_BUFFER_AND_PTR(m, c_size);

    if( crypto_aead_aes256gcm_decrypt_detached_afternm(m_ptr, NULL, c, c_size, mac, ad, ad_size, npub, (crypto_aead_aes256gcm_state*)ctx) == 0 ) {
        return info.GetReturnValue().Set(m);
    }

    return info.GetReturnValue().Set(Nan::Undefined());
}

/**
 * crypto_aead_aes256gcm_encrypt:
 * Encrypt Message in Combined Mode
 *
 *    var c = sodium.crypto_aead_aes256gcm_encrypt(
 *              message,
 *              additionalData,
 *              nonce,
 *              key);
 *
 * ~ message (Buffer): plain text buffer
 * ~ additionalData (Buffer): non-confidential data to add to the cipher text. Can be `null`
 * ~ nonce (Buffer): a nonce with `sodium.crypto_aead_aes256gcm_NPUBBYTES` in length
 * ~ key (Buffer): secret key `sodium.crypto_aead_aes256gcm_KEYBYTES` in length
 *
 * **Returns**:
 *
 * ~ cipherText (Buffer): The encrypted message, as well as a tag authenticating
 *   both the confidential message `message` and non-confidential data `additionalData`
 * ~ undefined: if `message` fails to encrypt
 *
 * **Sample**:
 *
 *     var sodium = require('sodium').api;
 *
 *     // Generate a random key
 *     var key = new Buffer(crypto_aead_aes256gcm_KEYBYTES);
 *     sodium.randombytes_buf(key);
 *
 *     // Generate random nonce
 *     var nonce = new Buffer(crypto_aead_aes256gcm_KEYBYTES);
 *     sodium.randombytes_buf(nonce);
 *
 *     var message = new Buffer("this is a plain text message");
 *     var additionalData = new Buffer("metadata");
 *
 *     // Encrypt Data
 *     var cipherText = sodium.crypto_aead_aes256gcm_encrypt(
 *        message, additionalData, nonce, key);
 *
 *     // Get the plain text, i.e., original message back
 *     var plainText = sodium.crypto_aead_aes256gcm_decrypt(
 *        cipherText, additionalData, nonce, key);
 */

 /**
  * crypto_aead_aes256gcm_decrypt:
  * Encrypt Message in Combined Mode
  *
  *    var m = sodium.crypto_aead_aes256gcm_decrypt(
  *              cipherText,
  *              additionalData,
  *              nonce,
  *              key);
  *
  * ~ cipherText (Buffer): encrypted buffer
  * ~ additionalData (Buffer): non-confidential data to add to the cipher text. Can be `null`
  * ~ nonce (Buffer): a nonce with `sodium.crypto_aead_aes256gcm_NPUBBYTES` in length
  * ~ key (Buffer): secret key `sodium.crypto_aead_aes256gcm_KEYBYTES` in length
  *
  * **Returns**:
  *
  * ~ plainText (Buffer): The decrypted plain text message
  * ~ undefined: if `message` fails to encrypt
  *
  * **See**: [crypto_aead_aes256gcm_encrypt](#crypto_aead_aes256gcm_encrypt)
  */
CRYPTO_AEAD_DEF(aes256gcm)

/**
 * crypto_aead_aes256gcm_encrypt_detached:
 * Encrypt Message in Detached Mode
 *
 *    var c = sodium.crypto_aead_aes256gcm_encrypt_detached(
 *              message,
 *              additionalData,
 *              nonce,
 *              key);
 *
 * ~ message (Buffer): plain text buffer
 * ~ additionalData (Buffer): non-confidential data to add to the cipher text. Can be `null`
 * ~ nonce (Buffer): a nonce with `sodium.crypto_aead_aes256gcm_NPUBBYTES` in length
 * ~ key (Buffer): secret key `sodium.crypto_aead_aes256gcm_KEYBYTES` in length
 *
 * **Returns**:
 *
 * ~ object: `cipherText` buffer with ciphered text, and `mac` buffer with the
 *   authentication tag
 *
 *    { cipherText: <buffer>, mac: <buffer> }
 *
 * ~ undefined: if `message` fails to encrypt
 *
 * **Sample**:
 *
 *     var sodium = require('sodium').api;
 *
 *     // Generate a random key
 *     var key = new Buffer(crypto_aead_aes256gcm_KEYBYTES);
 *     sodium.randombytes_buf(key);
 *
 *     // Generate random nonce
 *     var nonce = new Buffer(crypto_aead_aes256gcm_KEYBYTES);
 *     sodium.randombytes_buf(nonce);
 *
 *     var message = new Buffer("this is a plain text message");
 *     var additionalData = new Buffer("metadata");
 *
 *     // Encrypt Data
 *     var c = sodium.crypto_aead_aes256gcm_encrypt_detached(
 *        message, additionalData, nonce, key);
 *
 *     // Get the plain text, i.e., original message back
 *     var plainText = sodium.crypto_aead_aes256gcm_decrypt_detached(
 *        c.cipherText, c.mac, nonce, key);
 */

 /**
  * crypto_aead_aes256gcm_decrypt_detached:
  * Encrypt Message in Detached Mode
  *
  *    var m = sodium.crypto_aead_aes256gcm_decrypt(
  *              cipherText,
  *              mac,
  *              nonce,
  *              key);
  *
  * ~ cipherText (Buffer): encrypted buffer
  * ~ mac (Buffer): authentication tag. Can be `null`
  * ~ nonce (Buffer): a nonce with `sodium.crypto_aead_aes256gcm_NPUBBYTES` in length
  * ~ key (Buffer): secret key `sodium.crypto_aead_aes256gcm_KEYBYTES` in length
  *
  * **Returns**:
  *
  * ~ plainText (Buffer): The decrypted plain text message
  * ~ undefined: if `message` fails to encrypt
  *
  * **See**: [crypto_aead_aes256gcm_encrypt](#crypto_aead_aes256gcm_encrypt)
  */
CRYPTO_AEAD_DETACHED_DEF(aes256gcm)

/**
 * crypto_aead_chacha20poly1305_encrypt:
 * Encrypt Message in Detached Mode using ChaCha20-Poly1305
 *
 * See [crypto_aead_aes256gcm_encrypt](#crypto_aead_aes256gcm_encrypt)
 */

/**
 * crypto_aead_chacha20poly1305_decrypt:
 * Dencrypt Message in Detached Mode using ChaCha20-Poly1305
 *
 * See [crypto_aead_aes256gcm_decrypt](#crypto_aead_aes256gcm_decrypt)
 */
CRYPTO_AEAD_DEF(chacha20poly1305)

/**
 * crypto_aead_chacha20poly1305_encrypt_detached:
 * Encrypt Message in Detached Mode using ChaCha20-Poly1305
 *
 * See [crypto_aead_aes256gcm_encrypt_detached](#crypto_aead_aes256gcm_encrypt_detached)
 */

/**
 * crypto_aead_chacha20poly1305_decrypt_detached:
 * Dencrypt Message in Detached Mode using ChaCha20-Poly1305
 *
 * See [crypto_aead_aes256gcm_decrypt_detached](#crypto_aead_aes256gcm_decrypt_detached)
 */
CRYPTO_AEAD_DETACHED_DEF(chacha20poly1305)

/**
 * crypto_aead_chacha20poly1305_ietf_encrypt:
 * Encrypt Message in Combined Mode using ChaCha20-Poly1305-IETF
 *
 * See [crypto_aead_aes256gcm_encrypt](#crypto_aead_aes256gcm_encrypt)
 */

/**
 * crypto_aead_chacha20poly1305_decrypt:
 * Dencrypt Message in Combined Mode using ChaCha20-Poly1305-IETF
 *
 * See [crypto_aead_aes256gcm_decrypt](#crypto_aead_aes256gcm_decrypt)
 */
CRYPTO_AEAD_DEF(chacha20poly1305_ietf)

/**
 * crypto_aead_chacha20poly1305_ietf_encrypt_detached:
 * Encrypt Message in Detached Mode using ChaCha20-Poly1305-IETF
 *
 * See [crypto_aead_aes256gcm_encrypt_detached](#crypto_aead_aes256gcm_encrypt_detached)
 */

/**
 * crypto_aead_chacha20poly1305_decrypt_detached:
 * Dencrypt Message in Detached Mode using ChaCha20-Poly1305-IETF
 *
 * See [crypto_aead_aes256gcm_decrypt_detached](#crypto_aead_aes256gcm_decrypt_detached)
 */
CRYPTO_AEAD_DETACHED_DEF(chacha20poly1305_ietf)

/*
 * Register function calls in node binding
 */
void register_crypto_aead(Handle<Object> target) {
    NEW_METHOD(crypto_aead_aes256gcm_is_available);
    NEW_METHOD(crypto_aead_aes256gcm_beforenm);
    NEW_METHOD(crypto_aead_aes256gcm_encrypt_afternm);
    NEW_METHOD(crypto_aead_aes256gcm_decrypt_afternm);
    NEW_METHOD(crypto_aead_aes256gcm_encrypt_detached_afternm);
    NEW_METHOD(crypto_aead_aes256gcm_decrypt_detached_afternm);
    METHOD_AND_PROPS(aes256gcm);
    METHOD_AND_PROPS(chacha20poly1305);
    METHOD_AND_PROPS(chacha20poly1305_ietf);
}