/*
 * BIN/EVO Project 2017 - Celular automata art
 * Author: Filip Gulan (xgulan00)
 * Mail: xgulan00@stud.fit.vutbr.cz
 * Date: 7.5.2017
 */

#include "eca.h"

/**
 * Print help to stdin
 */
void printHelp()
{
    cout << "-------------------------------------------------------------------" << endl;
    cout << "BIN/EVO Project 2017 - Celular automata art" << endl;
    cout << "Author: Filip Gulan" << endl;
    cout << "E-mail: xgulan00@stud.fit.vutbr.cz" << endl;
    cout << "-------------------------------------------------------------------" << endl;
    cout << "Parameters:" << endl;
    cout << "   -f <path>: path of output file <path>. Default value: experiment.tab" << endl;
    cout << "   -x <path>: path to input pattern <path>. Required!" << endl;
    cout << "   -p <number>: population size. Default value: 20" << endl;
    cout << "   -g <number>: number of generations. Default value: 500" << endl;
    cout << "   -m <number>: mutation probability in percent. Default value: 10" << endl;
    cout << "   -s <number>: number of cells states. Default value: 10" << endl;
    cout << "   -r <number>: number of automata rules. Default value: 200" << endl;
    cout << "   -b: Mode for statistics, print only max fitness value on stdin" << endl;
    cout << "   -h: print this help" << endl;
    cout << "-------------------------------------------------------------------" << endl;
}

/**
 * Generate new rule
 * @param statesCount number of cells states
 * @return new rule
 */
rule generateRule(int statesCount)
{
    rule newRule;
    newRule.next = rand() % statesCount;
    newRule.current = rand() % statesCount;
    newRule.sum = rand() % (statesCount + statesCount +  statesCount + statesCount);
    return newRule;
}

/**
 * Generate chromozome, aka set of rules
 * @param statesCount number of cells states
 * @param rulesCount number of rules states
 * @return new chromozome
 */
chromozome generateChromozome(int statesCount, int rulesCount)
{
    chromozome newChromozome;
    newChromozome.fitness = EMPTY;
    for(int i = 0; i < rulesCount; i++)
    {
        rule newRule = generateRule(statesCount);
        newChromozome.rules.push_back(newRule);
    }
    return newChromozome;
}

/**
 * Generate population, aka set of chromozomes
 * @param populationSize population size
 * @param statesCount number of cells states
 * @param rulesCount number of rules
 * @return new population
 */
vector<chromozome> generatePopulation(int populationSize, int statesCount, int rulesCount)
{
    vector<chromozome> population;
    for(int i = 0; i < populationSize; i++)
    {
        chromozome newIndividual = generateChromozome(statesCount, rulesCount);
        population.push_back(newIndividual);
    }
    return population;
}

/**
 * Compare rules, rule1 with rule2
 * @param rule1
 * @param rule2
 * @return true if equal, otherwise false
 */
bool compareRules(rule rule1, rule rule2)
{
    if(rule1.sum == rule2.sum && rule1.current == rule2.current && rule1.next == rule2.next)
    {
        return true;
    }
    return false;
}

/**
 * Compare if rule1 is antirule to rule2
 * @param rule1
 * @param rule2
 * @return true if rule1 is antirule of rule2, otherwise false
 */
bool compareAntiRules(rule rule1, rule rule2)
{
    if(rule1.sum == rule2.sum && rule1.current == rule2.current && rule1.next != rule2.next)
    {
        return true;
    }
    return false;
}

/**
 * Get if rule1 is in rules
 * @param rules
 * @param rule1
 * @return true if rule1 is in rules, false otherwise
 */
bool isInRules(vector<rule> rules, rule rule1)
{
    int count = 0;
    for(int i = 0; i < rules.size(); i++)
    {
        if(compareRules(rules[i], rule1))
        {
            count++;
            if(count == 2)
            {
                return true;
            }
        }
    }
    return false;
}

/**
 * Get if rule1 has antirule in rules
 * @param rules
 * @param rule1
 * @return true if rule1 has antirule in rules, false otherwise
 */
bool isInAntiRules(vector<rule> rules, rule rule1)
{
    int count = 0;
    for(int i = 0; i < rules.size(); i++)
    {
        if(compareAntiRules(rules[i], rule1))
        {
            count++;
            if(count == 2)
            {
                return true;
            }
        }
    }
    return false;
}

/**
 * Get 1d index in 2d matrix
 * @param i position
 * @param j position
 * @param width of matrix
 * @return real 1d position
 */
int getIndex(int i, int j, int width)
{
    if(j >= width) //edge
    {
        j = 0;
    }
    if(i >= width) //edge
    {
        i = 0;
    }
    if(j < 0) //edge
    {
        j = width - 1;
    }
    if(i < 0) //edge
    {
        i = width - 1;
    }
    return j + i * width;
}

/**
 * Apply rule to cell
 * @param rules
 * @param current value of cell
 * @param sum of neighbours
 * @return new state of cell
 */
int applyRule(vector<rule> rules, int current, int sum)
{
    for(int i = 0; i < rules.size(); i++)
    {
        if(rules[i].sum == sum && rules[i].current == current)
        {
            return rules[i].next;
        }
    }
    return current;
}

/**
 * Evolve automata
 * @param space of cells
 * @param rules
 * @param width of automata
 * @return new automata space
 */
vector<int> evolve(vector<int> space, vector<rule> rules, int width)
{
    vector<int> tempSpace;
    for(int i = 0; i < width * width; i++) //initialization
    {
        tempSpace.push_back(0);
    }
    for(int i = 0; i < width; i++) //row
    {
        for(int j = 0; j < width; j++) //col
        {
            int current = space[getIndex(i, j, width)];
            int left = space[getIndex(i - 1, j, width)];
            int right = space[getIndex(i + 1, j, width)];
            int top = space[getIndex(i, j - 1, width)];
            int bottom = space[getIndex(i, j + 1, width)];
            int sum = left + right + top + bottom;
            tempSpace[getIndex(i, j, width)] = applyRule(rules, current, sum);
        }
    }
    return tempSpace;
}

/**
 * Simulate celular automata computation
 * @param rules
 * @param statesCount number of cells states
 * @param width of automata space
 * @return finished space
 */
vector<int> simulator(vector<rule> rules, int statesCount, int width)
{
    vector<int> space;
    for(int i = 0; i < width * width; i++) //initialization
    {
        space.push_back(0);
    }
    space[getIndex(width/2, width/2, width)] = 1;
    //Evolve
    for(int i = 0; i < width; i++) //generations
    {
        space = evolve(space, rules, width);
    }
    return space;
}

/**
 * Function to compute fitness of chromozome
 * @param rules
 * @param statesCount number of states
 * @param pattern to compare with
 * @return computed fitness
 */
int fitness(vector<rule> rules, int statesCount, vector<int> pattern)
{
    int fitness = 0;
    int width = sqrt(pattern.size());
    vector<int> space = simulator(rules, statesCount, width);
    for(int i = 0; i < rules.size(); i++) //check if rules contains any natirules
    {
        if(isInAntiRules(rules, rules[i]))
        {
            return fitness;
        }
    }
    for(int i = 0; i < width; i++) //row
    {
        for(int j = 0; j < width; j++) //col
        {
            if(pattern[getIndex(i, j, width)] != 0 && space[getIndex(i, j, width)] != 0)
            {
                fitness++;
            }
            else if(pattern[getIndex(i, j, width)] == 0 && space[getIndex(i, j, width)] != 0)
            {
                fitness--;
            }
        }
    }
    return fitness;
}

/**
 * Rate whole population of chromozomes
 * @param population of chromozomes
 * @param statesCount number of states
 * @param pattern desired result
 * @return rated population
 */
vector<chromozome> ratePopulation(vector<chromozome> population, int statesCount, vector<int> pattern)
{
    for(int i = 0; i < population.size(); i++)
    {
        population[i].fitness = fitness(population[i].rules, statesCount, pattern);
    }
    return population;
}

/**
 * Computation of new value in mutation
 * @param oldValue before mutation
 * @param direction 1/-1
 * @param max of mutation
 * @return new computed value
 */
int mutationNewValue(int oldValue, int direction, int max)
{
    int newValue = 0;
    if(oldValue + direction >= max) //edge
    {
        newValue = 0;
    }
    else if(oldValue + direction < 0) //edge
    {
        newValue = max - 1;
    }
    else
    {
        newValue = oldValue + direction;
    }
    return newValue;
}

/**
 * Mutation of chromozome
 * @param individual chromozome
 * @param mutationProbabilit in percent
 * @param statesCount number of cell states
 * @return mutated chromozome
 */
chromozome mutate(chromozome individual, int mutationProbabilit, int statesCount)
{
    for(int i = 0; i < individual.rules.size(); i++)
    {
        int random = rand() % 100;
        int direction = (rand() % 3) - 1;
        if(random < mutationProbabilit) //mutation of current
        {
            individual.rules[i].current = mutationNewValue(individual.rules[i].current, direction, statesCount);
        }
        random = rand() % 100;
        direction = (rand() % 3) - 1;
        if(random < mutationProbabilit) //mutation of next
        {
            individual.rules[i].next = mutationNewValue(individual.rules[i].next, direction, statesCount);
        }
        random = rand() % 100;
        direction = (rand() % 3) - 1;
        if(random < mutationProbabilit) //mutation of sum
        {
            individual.rules[i].sum = mutationNewValue(individual.rules[i].sum, direction, statesCount + statesCount +  statesCount + statesCount);
        }
    }
    return individual;
}

/**
 * Cross breeding of chromozomes, parent1 with parent2
 * @param parent1 chromozome
 * @param parent2 chromozome
 * @param statesCount number of cells states
 * @param rulesCount number of rules
 * @return new crossed chromozome
 */
chromozome cross(chromozome parent1, chromozome parent2, int statesCount, int rulesCount)
{
    chromozome child = generateChromozome(statesCount, rulesCount);
    int randomCross = rand() % (parent1.rules.size());
    for(int i = 0; i < child.rules.size(); i++)
    {
        if(randomCross < i) //get rules from parent1
        {
            child.rules[i].current = parent1.rules[i].current;
            child.rules[i].next = parent1.rules[i].next;
            child.rules[i].sum = parent1.rules[i].sum;
        }
        else //get rules from parent 2
        {
            child.rules[i].current = parent2.rules[i].current;
            child.rules[i].next = parent2.rules[i].next;
            child.rules[i].sum = parent2.rules[i].sum;
        }
    }
    return child;
}

/**
 * Compare by fitness
 * @param a chromozome
 * @param b chromozome
 * @return true if a is greater than b, false otherwise
 */
bool compareByFitness(const chromozome &a, const chromozome &b)
{
    return a.fitness > b.fitness;
}

/**
 * Whole process of Genetics algorithm evolution
 * @param populationSize size of population
 * @param generationCount number of generation
 * @param mutationProbability in percent
 * @param statesCount number of cells states
 * @param rulesCount number of rules
 * @param pattern desired result
 * @return best chromozome aka solution
 */
chromozome evolution(int populationSize, int generationCount, int mutationProbability, int statesCount, int rulesCount, vector<int> pattern, bool statistics)
{
    //Random generate population
    vector<chromozome> population = generatePopulation(populationSize, statesCount, rulesCount);
    population = ratePopulation(population, statesCount, pattern);
    sort(population.begin(), population.end(), compareByFitness);

    int parentsCount = population.size()/3;
    int childrenCount = 2*population.size()/3 + 1; // +1 is important!!!!

    for(int i = 0; i < generationCount; i++)
    {
        vector<chromozome> tempPopulation;
        tempPopulation.clear();

        for(int i = 0; i < parentsCount; i++) //pick best chromozomes from current population
        {
            tempPopulation.push_back(population[i]);
        }

        for(int i = 0; i < childrenCount; i++)
        {
            //cross
            chromozome parent1 = population[rand() % (population.size()/2)];
            chromozome parent2 = population[rand() % (population.size()/2)];
            chromozome child = cross(parent1, parent2, statesCount, rulesCount);
            //mutate
            child = mutate(child, mutationProbability, statesCount);
            //insert
            tempPopulation.push_back(child);
        }

        population.clear();
        for(int i = 0; i < tempPopulation.size(); i++) //replace population with new ones
        {
            population.push_back(tempPopulation[i]);
        }
        population = ratePopulation(population, statesCount, pattern);
        sort(population.begin(), population.end(), compareByFitness);
        if(!statistics)
        {
            cout << "Generatio: " << i + 1 << "of" << generationCount << " Best fitness: " << population[BEST].fitness << endl;
        }
    }
    if(statistics)
    {
        cout << population[BEST].fitness << endl;
    }
    return population[BEST];
}

/**
 * Generate final rule
 * @param left value
 * @param right value
 * @param top value
 * @param bottom value
 * @param current value
 * @param next value
 * @return final rule with all values
 */
finalRule generateFinalRule(int left, int right, int top, int bottom, int current, int next)
{
    finalRule newFinalRule;
    newFinalRule.left = left;
    newFinalRule.right = right;
    newFinalRule.top = top;
    newFinalRule.bottom = bottom;
    newFinalRule.current = current;
    newFinalRule.next = next;
    return newFinalRule;
}

/**
 * Convert our additive rule to non aditive rules for BiCAS
 * @param rule1 to be converted
 * @param statesCount number of states
 * @return non aditive rules
 */
vector<finalRule> aditiveToNonAditive(rule rule1, int statesCount)
{
    vector<finalRule> finalRules;
    for(int l = 0; l < statesCount; l++)
    {
        for(int r = 0; r < statesCount; r++)
        {
            for(int t = 0; t < statesCount; t++)
            {
                for(int b = 0; b < statesCount; b++)
                {
                    if((l + r + t + b) == rule1.sum)
                    {
                        finalRules.push_back(generateFinalRule(l, r, t, b, rule1.current, rule1.next));
                    }
                }
            }
        }
    }

    return finalRules;
}

/**
 * Read patter from file and store into vector
 * @param fileName name of pattern file
 * @return stored pattern i vector
 */
vector<int> readPattern(string fileName, int *status)
{
    vector<int> pattern;
    string line;
    int number;
    ifstream file (fileName);
    if(!file.is_open())
    {
        *status = FAILURE;
        return pattern;
    }
    while ( getline (file, line) ) //read by line
    {
        istringstream lineStream(line);
        while (lineStream >> number) //read by character
        {
            pattern.push_back(number);
        }
    }
    file.close();
    return pattern;
}

/**
 * Save solution to file
 * @param fileName name of file, where to save
 * @param statesCount number of states
 * @param solution founded solutiuon
 */
void saveSolution(string fileName, int statesCount, chromozome solution)
{
    fileName += ".tab";
    ofstream file;
    file.open(fileName.c_str());
    if(file.fail())
    {
        cerr << "Can not create file!" << endl;
    }
    else
    {
        file << statesCount << endl;
        for(int i = 0; i < solution.rules.size(); i++)
        {
            vector<finalRule> finalRules = aditiveToNonAditive(solution.rules[i], statesCount);
            for(int j = 0; j < finalRules.size(); j++)
            {
                file << finalRules[j].top << " " << finalRules[j].left << " " << finalRules[j].current << " " << finalRules[j].right << " " << finalRules[j].bottom << " " << finalRules[j].next << endl;
            }
        }
    }

}

/**
 * Main program function
 * @param argc number of parameters on command line
 * @param argv parameters on command line
 * @return 0 of success, 1 in case of error
 */
int main(int argc, char *argv[])
{
    //Parse comandline arguments
    string fileName = "experiment";
    string patternFileName = "";
    int populationSize = 20;
    int generationCount = 500;
    int mutationProbability = 10;
    int statesCount = 10;
    int rulesCount = 200;
    int seed = time(0);
    bool statistics = false;
    string lastParam = "";
    for(int i = 1; i < argc; i++)
    {
        if(lastParam.compare("") == 0) //we check for - parameters
        {
            if(strcmp(argv[i], "-p") == 0) //population size
            {
                lastParam = "-p";
            }
            else if(strcmp(argv[i], "-g") == 0) //number of generations
            {
                lastParam = "-g";
            }
            else if(strcmp(argv[i], "-m") == 0) //mutation probability
            {
                lastParam = "-m";
            }
            else if(strcmp(argv[i], "-s") == 0) //states count
            {
                lastParam = "-s";
            }
            else if(strcmp(argv[i], "-r") == 0) //rules count
            {
                lastParam = "-r";
            }
            else if(strcmp(argv[i], "-f") == 0) //file name
            {
                lastParam = "-f";
            }
            else if(strcmp(argv[i], "-x") == 0) //pattern file name
            {
                lastParam = "-x";
            }
            else if(strcmp(argv[i], "-b") == 0) //mode for statistics
            {
                lastParam = "-b";
                statistics = true;
                lastParam = "";
            }
            else if(strcmp(argv[i], "-h") == 0) //help
            {
                lastParam = "-h";
                lastParam = "";
                printHelp();
                return SUCCESS;
            }
            else //Unkown argument
            {
                cerr << ARG_ERR_MSG << endl;
                return FAILURE;
            }
        }
        else if(lastParam.compare("-p") == 0) //population size
        {
            populationSize =  atoi(argv[i]);
            lastParam = "";
        }
        else if(lastParam.compare("-g") == 0) //number of generations
        {
            generationCount =  atoi(argv[i]);
            lastParam = "";
        }
        else if(lastParam.compare("-m") == 0) //mutation probability
        {
            mutationProbability =  atoi(argv[i]);
            lastParam = "";
        }
        else if(lastParam.compare("-s") == 0) //states count
        {
            statesCount =  atoi(argv[i]);
            lastParam = "";
        }
        else if(lastParam.compare("-r") == 0) //rules count
        {
            rulesCount =  atoi(argv[i]);
            lastParam = "";
        }
        else if(lastParam.compare("-f") == 0) //input file path
        {
            fileName = argv[i];
            lastParam = "";
        }
        else if(lastParam.compare("-x") == 0) //input pattern file path
        {
            patternFileName = argv[i];
            lastParam = "";
        }
    }
    if(patternFileName.compare("") == 0)
    {
        cerr << ARG_ERR_MSG << endl;
        return FAILURE;
    }
    srand(seed);
    int status = SUCCESS;
    vector<int> pattern = readPattern(patternFileName, &status);
    if(status == FAILURE)
    {
        cerr << "Patter file can not be opened!" << endl;
        return  FAILURE;
    }
    if(!statistics)
    {
        cout << "Output file: " + fileName + ".tab" << endl;
        cout << "Population:" << populationSize << " Generation:" << generationCount << " Mutation:" << mutationProbability << " StatesCount:" << statesCount << " RulesCount:" << rulesCount << " Seed:" << seed << endl;
    }
    chromozome solution = evolution(populationSize, generationCount, mutationProbability, statesCount, rulesCount, pattern, statistics);
    saveSolution(fileName, statesCount, solution);
    return SUCCESS;
}