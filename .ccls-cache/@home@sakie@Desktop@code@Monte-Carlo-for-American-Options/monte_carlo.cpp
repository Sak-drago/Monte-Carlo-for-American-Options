#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>
#include <algorithm> // for max
#include <string>
#include <sstream>
#include <numeric>   // for accumulate

double callOptionPayOff(double S, double K);
double putOptionPayOff(double S, double K);
double mcAmericanOptionPricing(double S_0, double K, double r, double sigma, double T, int32_t nSimulations, int nTimeSteps, bool isCallOption);
double randomGaussian(double mean, double sigma);
double convertTime(std::string &expiry);

// Gaussian random number generator
double randomGaussian(double mean, double sigma) {
    static std::mt19937 generator(std::random_device{}());
    std::normal_distribution<double> distribution(mean, sigma);
    return distribution(generator);
}

// Payoff functions
double callOptionPayOff(double S, double K) {
    return std::max(S - K, 0.0);
}

double putOptionPayOff(double S, double K) {
    return std::max(K - S, 0.0);
}

// Regression function to estimate continuation value
double leastSquaresRegression(const std::vector<double>& X, const std::vector<double>& Y) {
    int n = X.size();
    double S_XY = 0.0, S_X = 0.0, S_Y = 0.0, S_X2 = 0.0;

    for (int i = 0; i < n; ++i) {
        S_X += X[i];
        S_Y += Y[i];
        S_XY += X[i] * Y[i];
        S_X2 += X[i] * X[i];
    }

    double slope = (n * S_XY - S_X * S_Y) / (n * S_X2 - S_X * S_X);
    double intercept = (S_Y - slope * S_X) / n;

    return intercept;
}

double convertTime(std::string &expiry, int c_m, int c_y){
    int year, month;
    std::stringstream ss(expiry);
    std::string placeholder_m, placeholder_y;

    std::getline(ss,placeholder_y,'-');
    std::getline(ss,placeholder_m,'-');
    
    year = std::stoi(placeholder_y);
    month = std::stoi(placeholder_m);
    if(month-c_m>0 && year == c_y) return double((month-c_m)/10);
    else if(month-c_m<0 && year == c_y+1) return double((month-c_m)/10);
    else return year-c_y;

}

// Monte Carlo for American Option pricing using LSMC
double mcAmericanOptionPricing(double S_0, double K, double r, double sigma, double T, int32_t nSimulations, int nTimeSteps, bool isCallOption) {
    double dt = T / static_cast<double>(nTimeSteps);
    std::vector<std::vector<double>> pricePaths(nSimulations, std::vector<double>(nTimeSteps + 1));
    std::vector<std::vector<double>> payoffs(nSimulations, std::vector<double>(nTimeSteps + 1));

    for (int i = 0; i < nSimulations; ++i) {
        pricePaths[i][0] = S_0;
        for (int t = 1; t <= nTimeSteps; ++t) {
            double gauss_bm = randomGaussian(0.0, 1.0);
            pricePaths[i][t] = pricePaths[i][t - 1] * std::exp((r - 0.5 * sigma * sigma) * dt + sigma * std::sqrt(dt) * gauss_bm);
        }
    }

    for (int i = 0; i < nSimulations; ++i) {
        for (int t = 0; t <= nTimeSteps; ++t) {
            if (isCallOption) {
                payoffs[i][t] = callOptionPayOff(pricePaths[i][t], K);
            } else {
                payoffs[i][t] = putOptionPayOff(pricePaths[i][t], K);
            }
        }
    }

    for (int t = nTimeSteps - 1; t >= 1; --t) {
        std::vector<double> inTheMoneyPaths;
        std::vector<double> futureValues;

        // Collect in-the-money paths
        for (int i = 0; i < nSimulations; ++i) {
            if (payoffs[i][t] > 0) {
                inTheMoneyPaths.push_back(pricePaths[i][t]);
                futureValues.push_back(payoffs[i][t + 1] * std::exp(-r * dt)); // Discount future payoff
            }
        }

        if (inTheMoneyPaths.size() > 1) {
            // Step 4: Estimate continuation values using regression
            double continuationValue = leastSquaresRegression(inTheMoneyPaths, futureValues);

            // Compare immediate payoff with continuation value to decide exercise or not
            for (int i = 0; i < nSimulations; ++i) {
                if (payoffs[i][t] > continuationValue) {
                    payoffs[i][t + 1] = payoffs[i][t];
                }
            }
        }
    }

    double payoffSum = 0.0;
    for (int i = 0; i < nSimulations; ++i) {
        payoffSum += payoffs[i][nTimeSteps] * std::exp(-r * T); // Final discounted payoff
    }

    return payoffSum / static_cast<double>(nSimulations);
}

int main(int argc, char* argv[]) {
    if(argc != 0){
    std::cerr << "Error: " << argv[0] << " <Last Price><Strike><Volatility><Interest Rate><Expiry>\n";
    return -1;
  }
    double S_0 = std::atof(argv[1]);   // Stock price at time 0
    double K = std::atof(argv[2]);     // Strike price
    double r = std::atof(argv[4]);      // Risk-free rate
    double sigma = std::atoi(argv[3]);   // Volatility
    double T = convertTime(argv[5]);       // Time to maturity (1 year)
    int32_t nSimulations = 100000; // Number of simulations
    int nTimeSteps = 100; // Number of time steps

    double callOptionPrice = mcAmericanOptionPricing(S_0, K, r, sigma, T, nSimulations, nTimeSteps, true);
    double putOptionPrice = mcAmericanOptionPricing(S_0, K, r, sigma, T, nSimulations, nTimeSteps, false);

    std::cout << "American Call Option Price: " << callOptionPrice << std::endl;
    std::cout << "American Put Option Price: " << putOptionPrice << std::endl;
    std::cout << "====================================================\n" << std::endl;
    return 0;
}

