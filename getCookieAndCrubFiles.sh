# Define the User-Agent header for better compatibility
export USER_AGENT="Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"

# Define the file where the cookie will be saved
export COOKIE_FILE="yahoo_cookies.txt"

echo "Setup complete. Proceed to Step 1."


echo "--- Running Step 1: Getting Cookie ---"
curl -s -L -c "$COOKIE_FILE" -H "User-Agent: $USER_AGENT" https://fc.yahoo.com/ -o /dev/null
sleep 2
echo "Cookie file content ($COOKIE_FILE):"
cat $COOKIE_FILE


echo "--- Running Step 2: Getting Crumb Token ---"
export CRUMB=$(curl -s -b "$COOKIE_FILE" -H "User-Agent: $USER_AGENT" https://query2.finance.yahoo.com/v1/test/getcrumb)

sleep 2
echo "Crumb Token captured in \$CRUMB variable:"
echo "$CRUMB">yahoo_crumb.txt
echo $CRUMB

echo "--- Running Step 3: Getting TSLA Quote ---"

# URL encode the crumb token (necessary if it contains '/' or other special characters)
#export ENCODED_CRUMB=$(echo "$CRUMB" | xxd -plain | sed 's/\(..\)/%\1/g')

# Final request: send cookie and use the encoded crumb in the URL

curl -s -b "$COOKIE_FILE" -H "User-Agent: $USER_AGENT" "https://query2.finance.yahoo.com/v7/finance/quote?symbols=TSLA&crumb=$CRUMB"
