#include <bits/stdc++.h>
extern vector<vector<uint32_t>> arr;

// These values will come from the ATD structure
// Assuming there will be ATD for this
// For the UMON structure

float get_mu_value(uint32_t core, uint32_t a, uint32_t b)
{
    int U = arr[core][b - 1] - arr[core][a - 1];
    return (float)U / (float)(b - a);
}

pair<float, uint32_t> get_max_mu(uint32_t core, uint32_t alloc, uint32_t balance)
{
    float max_mu = 0;
    uint32_t min_way = 0;
    for (uint32_t way = 1; way <= balance; way++)
    {
        float mu = get_mu_value(core, alloc, alloc + way);
        if (mu > max_mu)
        {
            max_mu = mu;
            min_way = way;
        }
    }
    return {max_mu, min_way};
}

vector<uint32_t> partition_algorithm()
{
    int balance = NUM_WAY - NUM_CPUS;
    vector<uint32_t> allocations(NUM_CPUS, 1);
    vector<pair<float, uint32_t>> present_state(NUM_CPUS, 0);
    while (balance != 0)
    {
        for (uint32_t application = 0; application < NUM_CPUS; application++)
        {
            uint32_t alloc = allocations[application];
            present_state[application] = get_max_mu(application, alloc, balance);
        }
        uint32_t winner = 0;
        float max_mu = 0;
        for (uint32_t application = 0; application < NUM_CPUS; application++)
        {
            if (present_state[application].first > max_mu)
            {
                winner = application;
                max_mu = present_state[application].first;
            }
        }
        allocations[winner] += present_state[winner].second;
        balance -= present_state[winner].second;
    }
    return allocations;
}