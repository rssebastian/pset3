#include <cs50.h>
#include <stdio.h>
#include <string.h>

// Max number of candidates
#define MAX 9

// preferences[i][j] is number of voters who prefer i over j
int preferences[MAX][MAX];

// locked[i][j] means i is locked in over j
bool locked[MAX][MAX];

// Each pair has a winner, loser
typedef struct
{
    int winner;
    int loser;
}
pair;

// Array of candidates
string candidates[MAX];
pair pairs[MAX * (MAX - 1) / 2];

int pair_count;
int candidate_count;

// Added Global Variable for lock_pairs function
int tent_lock;

// Function prototypes
bool vote(int rank, string name, int ranks[]);
void record_preferences(int ranks[]);
void add_pairs(void);
void sort_pairs(void);
void lock_pairs(void);
void print_winner(void);
bool check_cycle(int loser);

int main(int argc, string argv[])
{
    // Check for invalid usage
    if (argc < 2)
    {
        printf("Usage: tideman [candidate ...]\n");
        return 1;
    }

    // Populate array of candidates
    candidate_count = argc - 1;
    if (candidate_count > MAX)
    {
        printf("Maximum number of candidates is %i\n", MAX);
        return 2;
    }
    for (int i = 0; i < candidate_count; i++)
    {
        candidates[i] = argv[i + 1];
    }

    // Clear graph of locked in pairs
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = 0; j < candidate_count; j++)
        {
            locked[i][j] = false;
        }
    }

    pair_count = 0;
    int voter_count = get_int("Number of voters: ");

    // Query for votes
    for (int i = 0; i < voter_count; i++)
    {
        // ranks[i] is voter's ith preference
        int ranks[candidate_count];

        // Query for each rank
        for (int j = 0; j < candidate_count; j++)
        {
            string name = get_string("Rank %i: ", j + 1);

            if (!vote(j, name, ranks))
            {
                printf("Invalid vote.\n");
                return 3;
            }
        }

        record_preferences(ranks);

        printf("\n");
    }

    add_pairs();
    sort_pairs();
    lock_pairs();
    print_winner();
    return 0;
}

// Update ranks given a new vote
bool vote(int rank, string name, int ranks[])
{
    // Check candidates array for matching name
    for (int i = 0; i < candidate_count; i++)
    {
        // Reminder: strcmp returns 0 if strings are exactly the same
        if (strcmp(name, candidates[i]) == 0)
        {
            // Ranks gets filled with index of matched candidate
            ranks[rank] = i;
            return true;
        }
    }
    return false;
}

// Update preferences given one voter's ranks
void record_preferences(int ranks[])
{
    // Tracker for the winner per voter
    for (int i = 0; i < candidate_count; i++)
    {
        // Tracker for every relative loser per winner
        // Candidate at ranks[i] beats candidate at ranks[j]
        for (int j = i + 1; j < candidate_count; j++)
        {
            preferences[ranks[i]][ranks[j]]++;
        }
    }
}

// Record pairs of candidates where one is preferred over the other
void add_pairs(void)
{
    // Checks for each candidate
    for (int i = 0; i < candidate_count; i++)
    {
        // Enables us to not check the same pair twice
        for (int j = i; j < candidate_count; j++)
        {
            // Passes if tie is present
            if (preferences[i][j] != preferences[j][i])
            {
                // Assign who is the winner and loser
                if (preferences[i][j] > preferences[j][i])
                {
                    pairs[pair_count].winner = i;
                    pairs[pair_count].loser = j;
                    pair_count++;
                }
                else
                {
                    pairs[pair_count].winner = j;
                    pairs[pair_count].loser = i;
                    pair_count++;
                }
            }
        }
    }
}

// Sort pairs in decreasing order by strength of victory
void sort_pairs(void)
{
    // Implemented using Bubble Sort, was unsure how to implement
    // Merge Sort without creating/returning new arrays and also
    // mutating the original pairs array
    int swap_counter = -1;
    do
    {
        swap_counter = 0;
        for (int i = 0; i < pair_count - 1; i++)
        {
            if (preferences[pairs[i].winner][pairs[i].loser] < preferences[pairs[i + 1].winner][pairs[i + 1].loser])
            {
                pair temp = pairs[i];
                pairs[i] = pairs[i + 1];
                pairs[i + 1] = temp;
                swap_counter++;
            }
        }
    }
    while (swap_counter != 0);
}

// Lock pairs into the candidate graph in order, without creating cycles
void lock_pairs(void)
{
    // For every pair in the pairs array
    for (int i = 0; i < pair_count; i++)
    {

        // Set variable to tentative winner
        // Check if loser would ever cycle back to the tentative winner
        tent_lock = pairs[i].winner;

        // If it passes the cycle check, the locked location will be
        // set to true
        if (!check_cycle(pairs[i].loser))
        {
            locked[pairs[i].winner][pairs[i].loser] = true;
        }
    }
}

// Print the winner of the election
void print_winner(void)
{
    // Checks for each candidate
    for (int i = 0; i < candidate_count; i++)
    {
        // Default sets candidate checked as true
        bool source = true;

        // Checks if any other candidate has beaten the ith candidate
        for (int j = 0; j < candidate_count; j++)
        {
            // If so, candidates[i] cannot be the source
            if (locked[j][i] == true)
            {
                source = false;
            }
        }

        // Prints the name of the candidate if they passed the above test
        // and source is kept as true
        if (source)
        {
            printf("%s\n", candidates[i]);
        }
    }
}

bool check_cycle(int loser)
{
    // Check if the loser beating the tentative winner is true
    // Returns true that this causes a cycle
    if (locked[loser][tent_lock] == true)
    {
        return true;
    }

    // Checks each candidate if the loser has a locked in win
    for (int i = 0; i < candidate_count; i++)
    {
        if (locked[loser][i] == true)
        {
            // Check if the loser eventually points to the tentative winner
            if (check_cycle(i) == true)
            {
                // Only return true if all possible "true's" have been checked
                // Otherwise, continue checking each candidate
                return true;
            }
        }
    }

    // Returns false, no cycle is created if all other tests don't return
    return false;
}

