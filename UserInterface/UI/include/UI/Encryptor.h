#pragma once
#include <cryptopp/modes.h>
#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <string>

namespace UI
{
    class Encryptor
    {
    public:
        Encryptor();
        ~Encryptor();
    
        void Encrypt(std::string& text);
        void Decrypt(std::string& encryptedText);
    
    private:
        byte _key[CryptoPP::AES::DEFAULT_KEYLENGTH];
        byte _iv[CryptoPP::AES::BLOCKSIZE];
    };
}
