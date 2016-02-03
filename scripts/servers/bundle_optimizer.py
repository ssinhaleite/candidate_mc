#!/usr/bin/python

import zmq
import json
import pycmc
from pycmc import BundleOptimizer, BundleOptimizerParameters, BundleOptimizerResult, BundleOptimizerEpsStrategy
from pycmc import PyOracle, PyOracleWeights

# {
#
#   "dims" : <int>, (number of dimensions)
#
#   "parameters" : (optional)
#   {
#       "lambda"  : <double>, (regularizer weight)
#       "steps"   : <int>,    (number of iterations of bundle method)
#       "min_eps" : <double>, (convergence threshold, meaning depends on
#                              eps_strategy)
#       "eps_strategy" : enum { EpsFromGap, EpsFromChange }
#   }
# }
INITIAL_REQ      = 0

CONTINUATION_REQ = 1

EVALUATE_RES     = 2

FINAL_RES        = 3

class BundleMethodServer:

    def __init__(self):

        # init oracle
        self.oracle = PyOracle()
        self.oracle.setEvaluateFunctor(self.evaluate)
        self.running = True

    # callback for bundle method
    def evaluate(self, w, value, gradient):

        self.socket.send(chr(EVALUATE_RES) + json.dumps({ "x" : [ x for x in w] }))
        reply = self.socket.recv()

        t = ord(reply[0])
        if t != CONTINUATION_REQ:
            raise "Error: expected client to send CONTINUATION_REQ (" + str(CONTINUATION_REQ) + ", sent " + str(t) + " instead"

        data = json.loads(reply[1:])
        value = data["value"]
        for i in range(self.dims):
            gradient[i] = data["gradient"][i]

    def run(self):

        print "Setting up zmq socket"

        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.REP)
        self.socket.bind("tcp://*:4711")

        print "Waiting for client..."

        request = self.socket.recv()

        t = ord(request[0])

        if t != INITIAL_REQ:
            raise "Error: expected client to send INITIAL_REQ (" + str(INITIAL_REQ) + ", sent " + str(t) + " instead"

        data = json.loads(request[1:])
        self.dims = data["dims"]

        print "Got initial request for optimization with " + str(self.dims) + " variables"

        parameters = BundleOptimizerParameters()

        if data.has_key("parameters"):

            if data["parameters"].has_key("lambda"):
                parameters.lambada = data["parameters"]["lambda"]
            if data["parameters"].has_key("steps"):
                parameters.steps = data["parameters"]["steps"]
            if data["parameters"].has_key("min_eps"):
                parameters.min_eps = data["parameters"]["min_eps"]
            if data["parameters"].has_key("eps_strategy"):
                if data["parameters"]["eps_strategy"] == "eps_from_gap":
                    parameters.eps_strategy = EpsStrategy.EpsFromGap
                elif data["parameters"]["eps_strategy"] == "eps_from_change":
                    parameters.eps_strategy = EpsStrategy.EpsFromChange
                else:
                    raise "Unknown eps strategy: " + str(data["parameters"]["eps_strategy"])

        self.bundle_method = BundleOptimizer(parameters)

        # start bundle method (which will start sending a response, finishes 
        # receiving a request)
        w = PyOracleWeights(self.dims)
        result = self.bundle_method.optimize(self.oracle, w)

        if result == BundleOptimizerResult.ReachedMinGap:
            print "Optimal solution found at " + str([x for x in w])
        elif result == BundleOptimizerResult.ReachedMaxSteps:
            print "Maximal number of iterations reached"
        else:
            print "Optimal solution NOT found"

        self.socket.send(chr(FINAL_RES) + json.dumps({"x" : [x for x in w] }))

if __name__ == "__main__":

    pycmc.setLogLevel(pycmc.LogLevel.Debug)

    bms = BundleMethodServer()
    bms.run()
