#include <iostream>

template <class T1, class T2>
struct kv_pair
{
    T1 key;
    T2 value;
    kv_pair<T1, T2>* next;
};

template <class T1, class T2>
class hash_map
{
private:
    uint32_t hash_size;
    kv_pair<T1, T2>** chain_array;

public:
    hash_map(uint32_t _hash_size = 64)
    {
        this->hash_size = _hash_size;
        chain_array = new kv_pair<T1, T2>*[hash_size];
        for (uint32_t i = 0; i < hash_size; i++) chain_array[i] = nullptr;
    }

    ~hash_map()
    {
        for (uint32_t i = 0; i < hash_size; i++)
        {
            if (!chain_array[i]) continue;
            kv_pair<T1, T2>* temp1 = chain_array[i];
            kv_pair<T1, T2>* temp2 = chain_array[i]->next;
            while (temp2)
            {
                delete temp1;
                temp1 = temp2;
                temp2 = temp2->next;
            }
            delete temp1;
        }
        delete[] chain_array;
    }

    bool insert(T1 key, T2 value)
    {
        uint32_t hash_val = hashlittle(&key, 4, 0x01010101);
        uint32_t array_pos = hash_val % hash_size;
        kv_pair<T1, T2>* chain = chain_array[array_pos];
        bool found = false;
        while (chain)
        {
            if (chain->key == key)
            {
                found = true;
                break;
            }
            chain = chain->next;
        }
        if (found)
        {
            return false; // Fail to insert the desiginated item
        }
        else
        {
            kv_pair<T1, T2>* new_item = new kv_pair<T1, T2>;
            new_item->key = key;
            new_item->value = value;
            new_item->next = chain_array[array_pos];
            chain_array[array_pos] = new_item;
            return true;
        }
    }

    T2* find(T1 key)
    {
        uint32_t hash_val = hashlittle(&key, 4, 0x01010101);
        uint32_t array_pos = hash_val % hash_size;
        kv_pair<T1, T2>* chain = chain_array[array_pos];
        bool found = false;
        while (chain)
        {
            if (chain->key == key)
            {
                return &(chain->value);
            }
            chain = chain->next;
        }
        return nullptr;
    }

};