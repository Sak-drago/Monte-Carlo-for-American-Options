#!/bin/bash

csv_file="AAPL_options_data.csv"  # Change name depending on the stock

# Extract relevant fields from the CSV
symbol=$(awk -F, 'NR==2 {gsub(/"/,""); print $1}' $csv_file)            # Extract contract symbol
last_price=$(awk -F, 'NR==2 {gsub(/"/,""); print $5}' $csv_file)         # Extract last price
strike=$(awk -F, 'NR==2 {gsub(/"/,""); print $3}' $csv_file)            # Extract strike price
volatility=$(awk -F, 'NR==2 {gsub(/"/,""); print $11}' $csv_file)        # Extract implied volatility
expiry=$(awk -F, 'NR==2 {gsub(/"/,""); print $15}' $csv_file)           # Extract expiration date

# Assuming the interest rate is fixed or predefined (set to 0.05 or 5% as an example)
interest_rate=0.05

# Log extracted data for verification
echo "Contract Symbol: $symbol"
echo "Last Price: $last_price"
echo "Strike Price: $strike"
echo "Volatility: $volatility"
echo "Interest Rate: $interest_rate"
echo "Expiration Date: $expiry"

# Pass values to the C++ Monte Carlo simulation program
make monte_carlo
./monte_carlo "$last_price" "$strike" "$volatility" "$interest_rate" "$expiry"

