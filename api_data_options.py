import yfinance as yf
import pandas as pd

def fetch_option_data(ticker):
    stock = yf.Ticker(ticker)

    expiration_dates = stock.options

    option_data = []

    for date in expiration_dates:
        option_chain = stock.option_chain(date)
        
        # Add call and put data to the list
        calls = option_chain.calls
        puts = option_chain.puts
        
        calls['expirationDate'] = date
        puts['expirationDate'] = date
        
        calls['ticker'] = ticker
        puts['ticker'] = ticker
        
        option_data.append(calls)
        option_data.append(puts)

    all_options = pd.concat(option_data, ignore_index=True)

    return all_options

def main():
    ticker = 'AAPL'  # Change this to the stock you want

    option_data = fetch_option_data(ticker)

    # Save to CSV
    option_data.to_csv(f'{ticker}_options_data.csv', index=False)
    print(f'Option data for {ticker} saved to {ticker}_options_data.csv')

if __name__ == "__main__":
    main()

