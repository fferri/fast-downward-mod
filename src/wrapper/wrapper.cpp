#include "wrapper.h"

Term::Term(std::string functor)
    : functor_(functor)
{
}

Term::Term(std::string functor, std::vector<std::string> args)
    : functor_(functor), args_(args)
{
}

std::string Term::functor() const
{
    return functor_;
}

std::string Term::arg(int i) const
{
    return args_[i];
}

int Term::arity() const
{
    return args_.size();
}

std::string Term::str() const
{
    if(args_.size())
    {
        std::stringstream r;
        r << functor_;
        r << "(";
        for(int i = 0; i < args_.size(); i++)
        {
            if(i) r << ",";
            r << args_[i];
        }
        r << ")";
        return r.str();
    }
    else
    {
        return functor_;
    }
}

bool Term::operator ==(const Term &o) const
{
    return functor_ == o.functor_ && args_ == o.args_;
}

bool Term::operator <(const Term &o) const
{
    return functor_ < o.functor_ || (functor_ == o.functor_ && args_ < o.args_);
}

FastDownwardWrapper::FastDownwardWrapper(std::string fastdownward_dir, std::string fastdownward_script, std::string python_executable)
    : fastdownward_dir_(fastdownward_dir), fastdownward_script_(fastdownward_script), python_executable_(python_executable)
{
}

bool FastDownwardWrapper::plan(std::string problem_file, int timeout, int kill_signal)
{
    std::vector<std::string> argvec;
    argvec.push_back("python");
    argvec.push_back(fastdownward_dir_ + "/" + fastdownward_script_);
    argvec.push_back(problem_file);
    argvec.push_back("--search-options");
    argvec.push_back("--search");
    argvec.push_back("astar(lmcut())");

    char **argv = new char*[argvec.size()];
    for(int i = 0; i < argvec.size(); i++)
    {
        argv[i] = strdup(argvec[i].c_str());
    }

    int kill_signall = SIGKILL; // for terminating planner if time exceeds

    pid_t intermediate_pid = fork();
    if(intermediate_pid == 0) {
        pid_t worker_pid = fork();
        if(worker_pid == 0) {
            execv(python_executable_.c_str(), argv);
            perror("failed: execv");
            _exit(0);
        }

        pid_t timeout_pid = fork();
        if(timeout_pid == 0) {
            sleep(timeout);
            _exit(0);
        }

        pid_t exited_pid = wait(NULL);
        if(exited_pid == worker_pid) {
            kill(timeout_pid, SIGKILL);
        } else {
            std::cerr << "timeout reached" << std::endl;
            kill(worker_pid, kill_signal);
        }
        wait(NULL); // Collect the other process
        _exit(0); // Or some more informative status
    }
    waitpid(intermediate_pid, 0, 0);
    std::cerr << "reading solution..." << std::endl;

    const char *sas_plan_em = (fastdownward_dir_ + "/sas_plan_em").c_str();
    std::ifstream infile(sas_plan_em);
    std::string line;
    bool state_flag = true;
    while(std::getline(infile, line))
    {
        if(line[0] == ';') continue;

        if(state_flag)
        {
            std::set<Term> state = parseState(line);
            std::cout << "state: ";
            for(std::set<Term>::const_iterator it = state.begin(); it != state.end(); ++it)
            {
                std::cout << (it != state.begin() ? ", " : "") << it->str();
            }
            std::cout << std::endl;
        }
        else
        {
            Term action = parseAction(line);
            std::cout << "action: " << action.str() << std::endl;
        }
        state_flag = !state_flag;
    }

    return true;
}

std::set<Term> FastDownwardWrapper::parseState(std::string line)
{
    std::set<Term> terms;
    std::string func, arg;
    std::vector<std::string> args;
    int st = 0;
    for(int i = 0; i < line.size(); i++)
    {
        char c = line[i];
        switch(st)
        {
        case 0:
            if(c == '(') st = 1;
            break;
        case 1:
            if(c == ')') st = 0;
            else if(c == '(') st = 2;
            else if(c == ' ')
            {
                if(func.size())
                {
                    terms.insert(Term(func, args));
                    func = "";
                    arg = "";
                    args.clear();
                }
            }
            else
            {
                func += c;
            }
            break;
        case 2:
            if(c == '(')
            {
                // illegal char
            }
            else if(c == ',')
            {
                args.push_back(arg);
                arg = "";
            }
            else if(c == ' ')
            {
            }
            else if(c == ')')
            {
                args.push_back(arg);
                terms.insert(Term(func, args));
                func = "";
                arg = "";
                args.clear();
                st = 1;
            }
            else
            {
                arg += c;
            }
        }
    }
    return terms;
}

Term FastDownwardWrapper::parseAction(std::string line)
{
    std::vector<std::string> tokens;
    std::string tok;
    int st = 0;
    for(int i = 0; i < line.size(); i++)
    {
        char c = line[i];
        switch(st)
        {
        case 0:
            if(c == '(') st = 1;
            break;
        case 1:
            if(c == ')')
            {
                st = 0;
                tokens.push_back(tok);
                tok = "";
            }
            else if(c == ' ')
            {
                tokens.push_back(tok);
                tok = "";
            }
            else
            {
                tok += c;
            }
            break;
        }
    }
    std::vector<std::string> args;
    for(int i = 1; i < tokens.size(); i++)
    {
        args.push_back(tokens[i]);
    }
    return Term(tokens[0], args);
}

// Configuration options

/*
 * Reset the state of the planner to its default settings.
 */
void FastDownwardWrapper::reset()
{
    // TODO
}

/*
 * Get a planner property.
 */
std::string FastDownwardWrapper::getPlannerProperty(const std::string &s)
{
    // TODO
    return "";
}

/*
 * Set a planner property.
 */
bool FastDownwardWrapper::setPlannerProperty(const std::string &var, const std::string &value)
{
    // TODO
    return false;
}

// Domain configuration - clear functions

/*
 * Clear the current domain.
 */
void FastDownwardWrapper::clearDomain()
{
    domain_.str("");
    //clearDomainSymbols();
    //clearDomainActions();
    //clearDomainProblems();
    //clearDomainInitialState();
}

/*
 * Clear the current domain symbols.
 */
void FastDownwardWrapper::clearDomainSymbols()
{
    //domainSymbols_.str("");
}

/*
 * Clear the current domain actions.
 */
void FastDownwardWrapper::clearDomainActions()
{
    //domainActions_.str("");
}

/*
 * Clear the current domain problems.
 */
void FastDownwardWrapper::clearDomainProblems()
{
    //domainProblems_.str("");
}

/*
 * Clear the currently cached databases.
 */
void FastDownwardWrapper::clearDomainInitialState()
{
    //domainInitialStates_.str("");
}

// Domain configuration - file access functions

/*
 * Attempt to load a PKS domain file.
 */
bool FastDownwardWrapper::loadDomain(const std::string &filename)
{
    std::ifstream file(filename.c_str());

    if(file)
    {
        clearDomain();
        domain_ << file.rdbuf();
        file.close();
        return true;
    }
    else
    {
        return false;
    }
}

/*
 * Attempt to load a PKS symbol file.
 */
bool FastDownwardWrapper::loadDomainSymbols(const std::string &file)
{
    return false;
}

/*
 * Attempt to load a PKS action file.
 */
bool FastDownwardWrapper::loadDomainActions(const std::string &file)
{
    return false;
}

/*
 * Attempt to load a PKS problem file.
 */
bool FastDownwardWrapper::loadDomainProblems(const std::string &file)
{
    return false;
}

/*
 * Attempt to load a PKS goal file.
 */
bool FastDownwardWrapper::loadDomainProblemGoal(const std::string &file, const std::string &str)
{
    return false;
}


/*
 * Attempt to load a set of PKS update rules from a file.
 */
bool FastDownwardWrapper::loadDomainProblemUpdateRules(const std::string &file, const std::string &str)
{
    return false;
}


/*
 * Attempt to load a set of PKS maintenance rules from a file.
 */
bool FastDownwardWrapper::loadDomainProblemMaintenanceRules(const std::string &file, const std::string &str)
{
    return false;
}


/*
 * Attempt to load a PKS initial state description from a file.
 */
bool FastDownwardWrapper::loadDomainProblemInitialState(const std::string &file, const std::string &str)
{
    return false;
}


/*
 * Attempt to load a PKS database state fact file.
 */
bool FastDownwardWrapper::loadDomainProblemInitialStateFacts(const std::string &file, const std::string &str)
{
    return false;
}

/*
 * Attempt to load a PKS database state file.
 */
bool FastDownwardWrapper::loadDomainInitialState(const std::string &file)
{
    return false;
}

/*
 * Attempt to load a PKS database state fact file.
 */
bool FastDownwardWrapper::loadDomainInitialStateFacts(const std::string &file)
{
    return false;
}

// Domain configuration -- string definition functions

/*
 * Attempt to define a PKS domain from the specified string.
 */
bool FastDownwardWrapper::defineDomain(const std::string &str)
{
    domain_ << str;
    return true;
}

/*
 * Attempt to define a set of PKS symbols from the specified string.
 */
bool FastDownwardWrapper::defineDomainSymbols(const std::string &str)
{
    domain_ << str;
    return true;
}

/*
 * Attempt to define a set of PKS actions from the specified string.
 */
bool FastDownwardWrapper::defineDomainActions(const std::string &str)
{
    domain_ << str;
    return true;
}

/*
 * Attempt to define a set of PKS problems from the specified string.
 */
bool FastDownwardWrapper::defineDomainProblems(const std::string &str)
{
    problem_ << str;
    return true;
}

/*
 * Attempt to define a PKS goal from the specified string.
 */
bool FastDownwardWrapper::defineDomainProblemGoal(const std::string &s, const std::string &t)
{
    // TODO
    //problem_ << str;
    return true;
}


/*
 * Attempt to define a set of PKS update rules from the specified string.
 */
bool FastDownwardWrapper::defineDomainProblemUpdateRules(const std::string &s, const std::string &t)
{
    // TODO
    //problem_ << str;
    return true;
}


/*
 * Attempt to define a set of PKS maintenance rules from the specified string.
 */
bool FastDownwardWrapper::defineDomainProblemMaintenanceRules(const std::string &s, const std::string &t)
{
    // TODO
    //problem_ << str;
    return true;
}


/*
 * Attempt to define a PKS database state from a string.
 */
bool FastDownwardWrapper::defineDomainProblemInitialState(const std::string &s, const std::string &t)
{
    // TODO
    //problem_ << str;
    return true;
}


/*
 * Attempt to define a PKS Kf database state from a string.
 */
bool FastDownwardWrapper::defineDomainProblemInitialStateFacts(const std::string &s, const std::string &t)
{
    // TODO
    //problem_ << str;
    return true;
}


/*
 * Attempt to define a PKS Kf database from a StatePropertyList structure.
 */
bool FastDownwardWrapper::defineDomainProblemInitialStateFactsFromList(const std::string &s, const std::vector<Term> &t)
{
    // TODO
    //problem_ << str;
    return true;
}

/*
 * Attempt to define a PKS database state from a string.
 */
bool FastDownwardWrapper::defineDomainInitialState(const std::string &str)
{
    problem_ << str;
    return true;
}

/*
 * Attempt to define a PKS database state from a string.
 */
bool FastDownwardWrapper::defineDomainInitialStateFacts(const std::string &str)
{
    problem_ << str;
    return true;
}

/*
 * Attempt to define a PKS Kf database from a StatePropertyList structure.
 */
bool FastDownwardWrapper::defineDomainInitialStateFactsFromList(const std::set<Term> &s)
{
    // TODO

    /*
    std::vector<pks::StateProperty> state;

    statePropertyListIceToPks(s, state);

    return(planner->defineDomainInitialStateFacts(state));
    */

    //problem_ << wtf;

    return true;
}

// Planning functions

/*
 * Clear the current plan.
 */
void FastDownwardWrapper::clearPlan()
{
    plan_.clear();
    plan_valid_ = false;
    plan_states_.clear();
    plan_pointer_ = 0;
}

/*
 * Reset the current plan to the start of the plan.
 */
void FastDownwardWrapper::resetPlan()
{
    plan_pointer_ = 0;
}

/*
 * This function invokes the planner to build a new plan using the current
 * settings.
 */
bool FastDownwardWrapper::buildPlan()
{
    // TODO: build domain file

    // TODO: build problem file

    std::string problem_file = "";

    return plan(problem_file, 10);
}

/*
 * Return the current plan as a linear sequence of PlanSteps.
 */
std::vector<Term> FastDownwardWrapper::getPlan()
{
    return plan_;
}

/*
 * Return the current expected state as determined by the current plan.
 */
std::set<Term> FastDownwardWrapper::getPlanState()
{
    if(isEndOfPlan())
        return std::set<Term>();
    else
        return plan_states_[plan_pointer_ + 1];
}

/*
 * Return the next action in the plan. Note that subsequent calls to this
 * function do not advance the plan point.
 */
Term FastDownwardWrapper::getPlanAction()
{
    if(isEndOfPlan())
        return Term("EOP");
    else
        return plan_[plan_pointer_];
}


/*
 * Check if a plan is currently defined.
 */
bool FastDownwardWrapper::isPlanDefined()
{
    return plan_valid_;
}

/*
 * Check if the end of the current plan has been reached.
 */
bool FastDownwardWrapper::isEndOfPlan()
{
    return plan_pointer_ >= plan_.size();
}

/*
 * Advance to the next action in the plan.
 */
void FastDownwardWrapper::advancePlan()
{
    plan_pointer_++;
}

/*
 * Select a problem to use for planning.
 */
bool FastDownwardWrapper::setPlanProblem(const std::string &s)
{
    // TODO ?
    std::cerr << "setPlanProblem(\"" << s << "\") - not implemented" << std::endl;
    return false;
}

/*
 * Check if the preconditions of the next action in the plan are satisfied
 * in the specified state.
 */
bool FastDownwardWrapper::checkPlanActionPreconditions(const std::set<Term> &s)
{
    // TODO

    return true;
}

/*
 * Get the effects of the next action applied to the specified state. The
 * result is returned as a state difference structure.
 */
StateDelta FastDownwardWrapper::getPlanActionEffects(const std::set<Term> &s)
{
    // TODO

    StateDelta e;

    return e;
}

/*
 * Get the effects of the next action applied to the specified state. The
 * result is returned as a state.
 */
std::set<Term> FastDownwardWrapper::getPlanActionEffectsState(const std::set<Term> &s)
{
    // TODO

    std::set<Term> e;

    return e;
}

/*
 * Get the plan's initial state.
 */
std::set<Term> FastDownwardWrapper::getPlanInitialState()
{
    return plan_states_[0];
}

/*
 * Get the last defined state.
 */
std::set<Term> FastDownwardWrapper::getDomainInitialState()
{
    return plan_states_[0];
}

/*
 * Get the last defined problem state.
 */
std::set<Term> FastDownwardWrapper::getDomainProblemInitialState()
{
    return plan_states_[0];
}

/*
 * Get the difference between two states.
 */
StateDelta FastDownwardWrapper::getStateDifference(const std::set<Term> &s1, const std::set<Term> &s2)
{
    StateDelta diff;

    for(std::set<Term>::const_iterator it = s1.begin(); it != s1.end(); ++it)
    {
        if(s2.find(*it) == s2.end())
            diff.delList.insert(*it);
    }

    for(std::set<Term>::const_iterator it = s2.begin(); it != s2.end(); ++it)
    {
        if(s1.find(*it) == s1.end())
            diff.addList.insert(*it);
    }

    return diff;
}

int main()
{
    FastDownwardWrapper f("/home/federico/fast-downward");
    std::string prob = "/home/federico/fast-downward/benchmarks/gripper/prob01.pddl";
    f.plan(prob, 5);
}
