/**
 * @file device.cpp
 * @brief Chemical process simulation with Stream, Mixer and Reactor classes
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cmath>

using namespace std;

int streamcounter; ///< Global variable to keep track of stream creation.

const int MIXER_OUTPUTS = 1;
const float POSSIBLE_ERROR = 0.01;

/**
 * @class Stream
 * @brief Represents a chemical stream with a name and mass flow.
 */
class Stream
{
private:
    double mass_flow; ///< The mass flow rate of the stream.
    string name;      ///< The name of the stream.

public:
    /**
     * @brief Constructor to create a Stream with a unique name.
     * @param s An integer used to generate a unique name for the stream.
     */
    Stream(int s){setName("s"+std::to_string(s));}

    /**
     * @brief Set the name of the stream.
     * @param s The new name for the stream.
     */
    void setName(string s){name=s;}

    /**
     * @brief Get the name of the stream.
     * @return The name of the stream.
     */
    string getName(){return name;}

    /**
     * @brief Set the mass flow rate of the stream.
     * @param m The new mass flow rate value.
     */
    void setMassFlow(double m){mass_flow=m;}

    /**
     * @brief Get the mass flow rate of the stream.
     * @return The mass flow rate of the stream.
     */
    double getMassFlow() const {return mass_flow;}

    /**
     * @brief Print information about the stream.
     */
    void print() { cout << "Stream " << getName() << " flow = " << getMassFlow() << endl; }
};

/**
 * @class Device
 * @brief Represents a device that manipulates chemical streams.
 */
class Device
{
protected:
    vector<shared_ptr<Stream>> inputs;  ///< Input streams connected to the device.
    vector<shared_ptr<Stream>> outputs; ///< Output streams produced by the device.
    int inputAmount;
    int outputAmount;
public:
    /**
     * @brief Add an input stream to the device.
     * @param s A shared pointer to the input stream.
     */
    void addInput(shared_ptr<Stream> s){
        if(inputs.size() < inputAmount) inputs.push_back(s);
        else throw"INPUT STREAM LIMIT!";
    }
    /**
     * @brief Add an output stream to the device.
     * @param s A shared pointer to the output stream.
     */
    void addOutput(shared_ptr<Stream> s){
        if(outputs.size() < outputAmount) outputs.push_back(s);
        else throw "OUTPUT STREAM LIMIT!";
    }

    /**
     * @brief Update the output streams of the device (to be implemented by derived classes).
     */
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
        if (inputs.size() == _inputs_count) {
            throw "Too much inputs"s;
        }
        inputs.push_back(s);
    }
    void addOutput(shared_ptr<Stream> s) {
        if (outputs.size() == MIXER_OUTPUTS) {
            throw "Too much outputs"s;
        }
        outputs.push_back(s);
    }
    void updateOutputs() override {
        double sum_mass_flow = 0;
        for (const auto& input_stream : inputs) {
            sum_mass_flow += input_stream -> getMassFlow();
        }

        if (outputs.empty()) {
            throw "Should set outputs before update"s;
        }

        double output_mass = sum_mass_flow / outputs.size();

        for (auto& output_stream : outputs) {
            output_stream -> setMassFlow(output_mass);
        }
    }
};

void shouldSetOutputsCorrectlyWithOneOutput() {
    streamcounter=0;
    Mixer d1 = Mixer(2);

    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s2(new Stream(++streamcounter));
    shared_ptr<Stream> s3(new Stream(++streamcounter));
    s1->setMassFlow(10.0);
    s2->setMassFlow(5.0);

    d1.addInput(s1);
    d1.addInput(s2);
    d1.addOutput(s3);

    d1.updateOutputs();

    if (abs(s3->getMassFlow()) - 15 < POSSIBLE_ERROR) {
        cout << "Test 1 passed"s << endl;
    } else {
        cout << "Test 1 failed"s << endl;
    }
}

void shouldCorrectOutputs() {
    streamcounter=0;
    Mixer d1 = Mixer(2);

    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s2(new Stream(++streamcounter));
    shared_ptr<Stream> s3(new Stream(++streamcounter));
    shared_ptr<Stream> s4(new Stream(++streamcounter));
    s1->setMassFlow(10.0);
    s2->setMassFlow(5.0);

    d1.addInput(s1);
    d1.addInput(s2);
    d1.addOutput(s3);

    try {
        d1.addOutput(s4);
    } catch (const string ex) {
        if (ex == "Too much outputs"s) {
            cout << "Test 2 passed"s << endl;
            return;
        }
    }

    cout << "Test 2 failed"s << endl;
}

void shouldCorrectInputs() {
    streamcounter=0;
    Mixer d1 = Mixer(2);

    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s2(new Stream(++streamcounter));
    shared_ptr<Stream> s3(new Stream(++streamcounter));
    shared_ptr<Stream> s4(new Stream(++streamcounter));
    s1->setMassFlow(10.0);
    s2->setMassFlow(5.0);

    d1.addInput(s1);
    d1.addInput(s2);
    d1.addOutput(s3);

    try {
        d1.addInput(s4);
    } catch (const string ex) {
        if (ex == "Too much inputs"s) {
            cout << "Test 3 passed"s << endl;
            return;
        }
    }

    cout << "Test 3 failed"s << endl;
}

/**
 * @class Reactor
 * @brief Chemical reactor with 1 input and 1 or 2 outputs
 *
 * Reactor can operate in two modes:
 * - Single output mode: 1 input → 1 output
 * - Double output mode: 1 input → 2 outputs (mass split equally)
 */
class Reactor : public Device
{
private:
    bool isDoubleOutput; ///< Flag for double output mode

public:
    /**
     * @brief Constructor for Reactor device
     * @param isDoubleReactor true - 2 outputs, false - 1 output
     */
    Reactor(bool isDoubleReactor) : Device()
    {
        isDoubleOutput = isDoubleReactor;
        inputAmount = 1;  // Always 1 input
        outputAmount = isDoubleReactor ? 2 : 1;  // 1 or 2 outputs
    }

    /**
     * @brief Updates output streams based on input stream and reactor configuration
     *
     * In single output mode: output mass flow = input mass flow
     * In double output mode: each output gets half of input mass flow
     *
     * @throws std::string if outputs are not set before update
     */
    void updateOutputs() override
    {
        // Check if input is connected
        if (inputs.empty()) {
            throw "Input stream not connected to reactor"s;
        }

        // Check if outputs are set
        if (outputs.size() != outputAmount) {
            throw "Output streams not properly set for reactor"s;
        }

        double inputMass = inputs[0]->getMassFlow();

        if (isDoubleOutput) {
            // Split mass equally between two outputs
            double outputMass = inputMass / 2.0;
            outputs[0]->setMassFlow(outputMass);
            outputs[1]->setMassFlow(outputMass);
            cout << "Reactor: split mass " << inputMass << " into two outputs of " << outputMass << endl;
        } else {
            // Single output - same mass flow as input
            outputs[0]->setMassFlow(inputMass);
            cout << "Reactor: transferred mass " << inputMass << " to single output" << endl;
        }
    }

    /**
     * @brief Gets the reactor operation mode
     * @return true if reactor is in double output mode, false if single output
     */
    bool getIsDoubleOutput() const { return isDoubleOutput; }
};

/**
 * @test Test reactor in single output mode
 */
void testReactorSingleOutput() {
    cout << "=== Test 1: Reactor with single output ===" << endl;
    streamcounter = 0;
    Reactor reactor(false); // Single output mode

    shared_ptr<Stream> input(new Stream(++streamcounter));
    shared_ptr<Stream> output(new Stream(++streamcounter));

    input->setMassFlow(20.0);

    reactor.addInput(input);
    reactor.addOutput(output);
    reactor.updateOutputs();

    if (abs(output->getMassFlow() - 20.0) < POSSIBLE_ERROR) {
        cout << "PASS: Single output mode works correctly" << endl;
    } else {
        cout << "FAIL: Incorrect output mass" << endl;
    }
    cout << endl;
}

/**
 * @test Test reactor in double output mode
 */
void testReactorDoubleOutput() {
    cout << "=== Test 2: Reactor with double output ===" << endl;
    streamcounter = 0;
    Reactor reactor(true); // Double output mode

    shared_ptr<Stream> input(new Stream(++streamcounter));
    shared_ptr<Stream> output1(new Stream(++streamcounter));
    shared_ptr<Stream> output2(new Stream(++streamcounter));

    input->setMassFlow(30.0);

    reactor.addInput(input);
    reactor.addOutput(output1);
    reactor.addOutput(output2);
    reactor.updateOutputs();

    double totalOutput = output1->getMassFlow() + output2->getMassFlow();
    if (abs(totalOutput - 30.0) < POSSIBLE_ERROR &&
        abs(output1->getMassFlow() - 15.0) < POSSIBLE_ERROR) {
        cout << "PASS: Double output mode works correctly" << endl;
    } else {
        cout << "FAIL: Incorrect mass distribution" << endl;
    }
    cout << endl;
}

/**
 * @test Test reactor input limit
 */
void testReactorInputLimit() {
    cout << "=== Test 3: Reactor input limit ===" << endl;
    streamcounter = 0;
    Reactor reactor(false);

    shared_ptr<Stream> input1(new Stream(++streamcounter));
    shared_ptr<Stream> input2(new Stream(++streamcounter));
    shared_ptr<Stream> output(new Stream(++streamcounter));

    reactor.addInput(input1);
    try {
        reactor.addInput(input2);
        cout << "FAIL: Should not allow more than 1 input" << endl;
    } catch (const char* ex) {
        if (string(ex) == "INPUT STREAM LIMIT!") {
            cout << "PASS: Input limit enforced correctly" << endl;
        } else {
            cout << "FAIL: Wrong exception message" << endl;
        }
    }
    cout << endl;
}

/**
 * @test Test reactor output limit in single mode
 */
void testReactorOutputLimitSingle() {
    cout << "=== Test 4: Reactor output limit (single mode) ===" << endl;
    streamcounter = 0;
    Reactor reactor(false);

    shared_ptr<Stream> input(new Stream(++streamcounter));
    shared_ptr<Stream> output1(new Stream(++streamcounter));
    shared_ptr<Stream> output2(new Stream(++streamcounter));

    reactor.addInput(input);
    reactor.addOutput(output1);
    try {
        reactor.addOutput(output2);
        cout << "FAIL: Should not allow more than 1 output in single mode" << endl;
    } catch (const char* ex) {
        if (string(ex) == "OUTPUT STREAM LIMIT!") {
            cout << "PASS: Output limit enforced correctly" << endl;
        } else {
            cout << "FAIL: Wrong exception message" << endl;
        }
    }
    cout << endl;
}

void tests(){
    cout << "=== STARTING TESTS ===" << endl << endl;


    shouldSetOutputsCorrectlyWithOneOutput();
    shouldCorrectOutputs();
    shouldCorrectInputs();

    testReactorSingleOutput();
    testReactorDoubleOutput();
    testReactorInputLimit();
    testReactorOutputLimitSingle();

    cout << endl << "=== TESTS COMPLETED ===" << endl;
}

/**
 * @brief The entry point of the program.
 * @return 0 on successful execution.
 */
int main()
{
    cout << "Chemical Process Simulation Started" << endl;
    cout << "====================================" << endl;

    streamcounter = 0;
    tests();

    return 0;
}