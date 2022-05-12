#include "cache.h"

uint32_t CACHE::find_victim(uint32_t cpu, uint64_t instr_id, uint32_t set, const BLOCK *current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
    // baseline LRU replacement policy for other caches
    return lru_victim(cpu, instr_id, set, current_set, ip, full_addr, type);
}

void CACHE::update_replacement_state(uint32_t cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type, uint8_t hit)
{
    if (type == WRITEBACK)
    {
        if (hit) // wrietback hit does not update LRU state
            return;
    }

    return lru_update(set, way);
}

void CACHE::atd_lru_update(uint32_t set, uint32_t way, uint32_t cpu)
{
    /*
        Similar to lru_update
        Updates lru replacement state in the ATD of the given cpu
    */
    for (uint32_t i = 0; i < NUM_WAY; i++)
    {
        if (atd[cpu][set][i].lru < atd[cpu][set][way].lru) // Checking LRU state of ATD of given cpu
        {
            atd[cpu][set][i].lru++;
        }
    }
    atd[cpu][set][way].lru = 0; // promote to the MRU position
}

uint32_t CACHE::atd_lru_victim(uint32_t cpu, uint32_t set)
{
    uint32_t way = 0;

    // fill invalid line first
    for (way = 0; way < NUM_WAY; way++)
    {
        if (atd[cpu][set][way].valid == 0)
        {
            break;
        }
    }

    // LRU victim
    if (way == NUM_WAY)
    {
        for (way = 0; way < NUM_WAY; way++)
        {
            if (atd[cpu][set][way].lru == NUM_WAY - 1)
            {
                break;
            }
        }
    }

    if (way == NUM_WAY)
    {
        cerr << "[" << NAME << "] " << __func__ << " no victim! set: " << set << endl;
        assert(0);
    }

    return way;
}

uint32_t CACHE::lru_victim(uint32_t cpu, uint64_t instr_id, uint32_t set, const BLOCK *current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
    uint32_t way = 0;

    // fill invalid line first
    for (way = 0; way < NUM_WAY; way++)
    {
        if (block[set][way].valid == false)
        {

            DP(if (warmup_complete[cpu]) {
            cout << "[" << NAME << "] " << __func__ << " instr_id: " << instr_id << " invalid set: " << set << " way: " << way;
            cout << hex << " address: " << (full_addr>>LOG2_BLOCK_SIZE) << " victim address: " << block[set][way].address << " data: " << block[set][way].data;
            cout << dec << " lru: " << block[set][way].lru << endl; });

            break;
        }
    }

    // LRU victim
    if (way == NUM_WAY)
    {
        for (way = 0; way < NUM_WAY; way++)
        {
            if (block[set][way].lru == NUM_WAY - 1)
            {

                DP(if (warmup_complete[cpu]) {
                cout << "[" << NAME << "] " << __func__ << " instr_id: " << instr_id << " replace set: " << set << " way: " << way;
                cout << hex << " address: " << (full_addr>>LOG2_BLOCK_SIZE) << " victim address: " << block[set][way].address << " data: " << block[set][way].data;
                cout << dec << " lru: " << block[set][way].lru << endl; });

                break;
            }
        }
    }

    if (way == NUM_WAY)
    {
        cerr << "[" << NAME << "] " << __func__ << " no victim! set: " << set << endl;
        assert(0);
    }

    return way;
}

uint32_t CACHE::llc_lru_victim(uint32_t cpu, uint64_t instr_id, uint32_t set, const BLOCK *current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
    /*
        Similar to lru_victim
        Also checks the cpu attribute of the block
    */
    uint32_t way = 0;

    // fill invalid line first
    for (way = 0; way < NUM_WAY; way++)
    {
        if (block[set][way].cpu == cpu) // Checking CPU before chcecking checking for vaildity
        {
            if (block[set][way].valid == false)
            {
                DP(if (warmup_complete[cpu]) {
               cout << "[" << NAME << "] " << _func_ << " instr_id: " << instr_id << " invalid set: " << set << " way: " << way;
               cout << hex << " address: " << (full_addr>>LOG2_BLOCK_SIZE) << " victim address: " << block[set][way].address << " data: " << block[set][way].data;
               cout << dec << " lru: " << block[set][way].lru << endl; });

                return way;
                break;
            }
        }
    }

    // LRU victim
    if (way == NUM_WAY)
    {
        uint32_t lru_value = 0;
        uint32_t to_be_evicted = 0;
        for (way = 0; way < NUM_WAY; way++)
        {
            if (block[set][way].cpu == cpu) // Checking cpu before selecting max LRU value
            {
                if (lru_value <= block[set][way].lru) // Selectign max LRU value
                {
                    lru_value = block[set][way].lru; // choosing max LRU value
                    to_be_evicted = way;             // Selecting way to be evicted

                    DP(if (warmup_complete[cpu]) {
                    cout << "[" << NAME << "] " << _func_ << " instr_id: " << instr_id << " replace set: " << set << " way: " << way;
                    cout << hex << " address: " << (full_addr>>LOG2_BLOCK_SIZE) << " victim address: " << block[set][way].address << " data: " << block[set][way].data;
                    cout << dec << " lru: " << block[set][way].lru << endl; });
                }
            }
        }
        way = to_be_evicted;
    }

    if (way == NUM_WAY)
    {
        cerr << "[" << NAME << "] " << __func__ << " no victim! set: " << set << endl;
        assert(0);
    }
    return way;
}

void CACHE::lru_update(uint32_t set, uint32_t way)
{
    // update lru replacement state
    for (uint32_t i = 0; i < NUM_WAY; i++)
    {
        if (block[set][i].lru < block[set][way].lru)
        {
            block[set][i].lru++;
        }
    }
    block[set][way].lru = 0; // promote to the MRU position
}

void CACHE::llc_lru_update(uint32_t set, uint32_t way, uint32_t cpu)
{
    /*
        Similar to lru_update function
        updates lru replacement state of the partitioned LLC based of the cpu attribute of the block
    */
    for (uint32_t i = 0; i < NUM_WAY; i++)
    {
        if ((block[set][i].cpu == cpu) && (block[set][i].lru < block[set][way].lru)) // Checking cpu values before increasing LRU
        {
            block[set][i].lru++;
        }
    }
    block[set][way].lru = 0; // promote to the MRU position for the given cpu
}

void CACHE::replacement_final_stats()
{
}

#ifdef NO_CRC2_COMPILE
void InitReplacementState()
{
}

uint32_t GetVictimInSet(uint32_t cpu, uint32_t set, const BLOCK *current_set, uint64_t PC, uint64_t paddr, uint32_t type)
{
    return 0;
}

void UpdateReplacementState(uint32_t cpu, uint32_t set, uint32_t way, uint64_t paddr, uint64_t PC, uint64_t victim_addr, uint32_t type, uint8_t hit)
{
}

void PrintStats_Heartbeat()
{
}

void PrintStats()
{
}
#endif
