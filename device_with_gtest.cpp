/**
 * @file device_with_gtest.cpp
 * @brief Chemical process simulation with Google Tests
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cmath>

using namespace std;

int streamcounter;
const int MIXER_OUTPUTS = 1;
const float POSSIBLE_ERROR = 0.01;

// ==================== SIMPLE GTEST IMPLEMENTATION ====================
#include <stdexcept>
#include <vector>
#include <functional>

// Simple Google Test implementation
#define EXPECT_TRUE(condition) \
    if (!(condition)) { \
        std::cout << "Failure: " << #condition << " in " << __FILE__ << ":" << __LINE__ << std::endl; \
        throw std::runtime_error("Test failed"); \
    }

#define EXPECT_FALSE(condition) EXPECT_TRUE(!(condition))

#define EXPECT_NEAR(val1, val2, abs_error) \
    if (std::abs((val1) - (val2)) > (abs_error)) { \
        std::cout << "Failure: " << #val1 << " (" << (val1) << ") != " << #val2 << " (" << (val2) << ") in " << __FILE__ << ":" << __LINE__ << std::endl; \
        throw std::runtime_error("Test failed"); \
    }

#define EXPECT_THROW(statement, exception_type) \
    try { \
        statement; \
        std::cout << "Failure: Expected exception in " << __FILE__ << ":" << __LINE__ << std::endl; \
        throw std::runtime_error("Test failed"); \
    } catch (exception_type&) { \
        /* Expected */ \
    } catch (...) { \
        std::cout << "Failure: Wrong exception type in " << __FILE__ << ":" << __LINE__ << std::endl; \
        throw std::runtime_error("Test failed"); \
    }

// Test registration system
struct TestCase {
    std::string name;
    std::function<void()> function;
};

std::vector<TestCase>& GetTests() {
    static std::vector<TestCase> tests;
    return tests;
}

struct TestRegistrar {
    TestRegistrar(const std::string& name, std::function<void()> func) {
        GetTests().push_back({name, func});
    }
};

#define CONCAT_(a, b) a##b
#define CONCAT(a, b) CONCAT_(a, b)
#define TEST_REGISTRAR(test_suite, test_name) \
    static TestRegistrar CONCAT(reg, __LINE__)(#test_suite "." #test_name, [](){ test_suite##_##test_name##_Test::Run(); });

#define TEST(test_suite_name, test_name) \
    class test_suite_name##_##test_name##_Test { \
    public: \
        static void Run(); \
    }; \
    TEST_REGISTRAR(test_suite_name, test_name) \
    void test_suite_name##_##test_name##_Test::Run()

// ==================== END SIMPLE GTEST ====================

// ==================== КЛАССЫ ====================

class Stream
{
private:
    double mass_flow;
    string name;

public:
    Stream(int s){setName("s"+std::to_string(s));}
    void setName(string s){name=s;}
    string getName(){return name;}
    void setMassFlow(double m){mass_flow=m;}
    double getMassFlow() const {return mass_flow;}
    void print() { cout << "Stream " << getName() << " flow = " << getMassFlow() << endl; }
};

class Device
{
protected:
    vector<shared_ptr<Stream>> inputs;
    vector<shared_ptr<Stream>> outputs;
    int inputAmount;
    int outputAmount;

public:
    void addInput(shared_ptr<Stream> s){
        if((int)inputs.size() < inputAmount) inputs.push_back(s);  // FIX: cast to int
        else throw "INPUT STREAM LIMIT!";
    }

    void addOutput(shared_ptr<Stream> s){
        if((int)outputs.size() < outputAmount) outputs.push_back(s);  // FIX: cast to int
        else throw "OUTPUT STREAM LIMIT!";
    }

    virtual void updateOutputs() = 0;
};

class Mixer: public Device
{
private:
    int _inputs_count = 0;

public:
    Mixer(int inputs_count): Device() {
        _inputs_count = inputs_count;
    }

    void addInput(shared_ptr<Stream> s) {
        if ((int)inputs.size() == _inputs_count) {  // FIX: cast to int
            throw string("Too much inputs");  // FIX: use string instead of "text"s
        }
        inputs.push_back(s);
    }

    void addOutput(shared_ptr<Stream> s) {
        if ((int)outputs.size() == MIXER_OUTPUTS) {  // FIX: cast to int
            throw string("Too much outputs");  // FIX: use string
        }
        outputs.push_back(s);
    }

    void updateOutputs() override {
        double sum_mass_flow = 0;
        for (const auto& input_stream : inputs) {
            sum_mass_flow += input_stream->getMassFlow();
        }

        if (outputs.empty()) {
            throw string("Should set outputs before update");  // FIX: use string
        }

        double output_mass = sum_mass_flow / outputs.size();
        for (auto& output_stream : outputs) {
            output_stream->setMassFlow(output_mass);
        }
    }
};

class Reactor : public Device
{
private:
    bool isDoubleOutput;

public:
    Reactor(bool isDoubleReactor) : Device()
    {
        isDoubleOutput = isDoubleReactor;
        inputAmount = 1;
        outputAmount = isDoubleReactor ? 2 : 1;
    }

    void updateOutputs() override
    {
        if (inputs.empty()) {
            throw string("Input stream not connected to reactor");  // FIX: use string
        }

        if ((int)outputs.size() != outputAmount) {  // FIX: cast to int
            throw string("Output streams not properly set for reactor");  // FIX: use string
        }

        double inputMass = inputs[0]->getMassFlow();

        if (isDoubleOutput) {
            double outputMass = inputMass / 2.0;
            outputs[0]->setMassFlow(outputMass);
            outputs[1]->setMassFlow(outputMass);
        } else {
            outputs[0]->setMassFlow(inputMass);
        }
    }

    bool getIsDoubleOutput() const { return isDoubleOutput; }
};

// ==================== GOOGLE TESTS ====================

TEST(ReactorTest, SingleOutputMode) {
    streamcounter = 0;
    Reactor reactor(false);

    shared_ptr<Stream> input(new Stream(++streamcounter));
    shared_ptr<Stream> output(new Stream(++streamcounter));

    input->setMassFlow(20.0);

    reactor.addInput(input);
    reactor.addOutput(output);
    reactor.updateOutputs();

    EXPECT_NEAR(output->getMassFlow(), 20.0, POSSIBLE_ERROR);
}

TEST(ReactorTest, DoubleOutputMode) {
    streamcounter = 0;
    Reactor reactor(true);

    shared_ptr<Stream> input(new Stream(++streamcounter));
    shared_ptr<Stream> output1(new Stream(++streamcounter));
    shared_ptr<Stream> output2(new Stream(++streamcounter));

    input->setMassFlow(30.0);

    reactor.addInput(input);
    reactor.addOutput(output1);
    reactor.addOutput(output2);
    reactor.updateOutputs();

    EXPECT_NEAR(output1->getMassFlow(), 15.0, POSSIBLE_ERROR);
    EXPECT_NEAR(output2->getMassFlow(), 15.0, POSSIBLE_ERROR);

    // Check mass conservation
    double totalOutput = output1->getMassFlow() + output2->getMassFlow();
    EXPECT_NEAR(totalOutput, 30.0, POSSIBLE_ERROR);
}

TEST(ReactorTest, InputLimitEnforcement) {
    streamcounter = 0;
    Reactor reactor(false);

    shared_ptr<Stream> input1(new Stream(++streamcounter));
    shared_ptr<Stream> input2(new Stream(++streamcounter));

    reactor.addInput(input1);
    EXPECT_THROW(reactor.addInput(input2), const char*);
}

TEST(ReactorTest, OutputLimitSingleMode) {
    streamcounter = 0;
    Reactor reactor(false);

    shared_ptr<Stream> input(new Stream(++streamcounter));
    shared_ptr<Stream> output1(new Stream(++streamcounter));
    shared_ptr<Stream> output2(new Stream(++streamcounter));

    reactor.addInput(input);
    reactor.addOutput(output1);
    EXPECT_THROW(reactor.addOutput(output2), const char*);
}

TEST(ReactorTest, GetIsDoubleOutputMethod) {
    Reactor singleReactor(false);
    Reactor doubleReactor(true);

    EXPECT_FALSE(singleReactor.getIsDoubleOutput());
    EXPECT_TRUE(doubleReactor.getIsDoubleOutput());
}

TEST(ReactorTest, NoInputException) {
    streamcounter = 0;
    Reactor reactor(false);

    shared_ptr<Stream> output(new Stream(++streamcounter));
    reactor.addOutput(output);
    EXPECT_THROW(reactor.updateOutputs(), string);
}

TEST(ReactorTest, WrongOutputCountException) {
    streamcounter = 0;
    Reactor reactor(true);

    shared_ptr<Stream> input(new Stream(++streamcounter));
    shared_ptr<Stream> output1(new Stream(++streamcounter));

    reactor.addInput(input);
    reactor.addOutput(output1);
    EXPECT_THROW(reactor.updateOutputs(), string);
}

TEST(MixerTest, BasicFunctionality) {
    streamcounter = 0;
    Mixer mixer(2);

    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s2(new Stream(++streamcounter));
    shared_ptr<Stream> s3(new Stream(++streamcounter));

    s1->setMassFlow(10.0);
    s2->setMassFlow(5.0);

    mixer.addInput(s1);
    mixer.addInput(s2);
    mixer.addOutput(s3);
    mixer.updateOutputs();

    EXPECT_NEAR(s3->getMassFlow(), 15.0, POSSIBLE_ERROR);
}

// ==================== MAIN ====================

int main(int argc, char **argv) {
    cout << "Chemical Process Simulation - Google Tests" << endl;
    cout << "==========================================" << endl;

    // Run all tests using our simple test runner
    std::cout << "[==========] Running " << GetTests().size() << " tests" << std::endl;
    int passed = 0;
    int failed = 0;

    for (const auto& test : GetTests()) {
        std::cout << "[ RUN      ] " << test.name << std::endl;
        try {
            test.function();
            std::cout << "[       OK ] " << test.name << std::endl;
            passed++;
        } catch (const std::exception& e) {
            std::cout << "[  FAILED  ] " << test.name << std::endl;
            failed++;
        }
    }

    std::cout << "[==========] " << (passed + failed) << " tests ran" << std::endl;
    std::cout << "[  PASSED  ] " << passed << " tests" << std::endl;
    if (failed > 0) {
        std::cout << "[  FAILED  ] " << failed << " tests" << std::endl;
        return 1;
    }

    cout << endl << "All tests passed successfully!" << endl;
    return 0;
}