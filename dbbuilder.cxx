/* Take in the database name and two files as input, parse and build a database from them
 * Since this is a specific set of pre-formatted data, little checking needs to occur. */

#include <xapian.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>

// Remove instances of token from string
std::string strip(std::string item, char token) {

    item.erase(std::remove(item.begin(), item.end(), token), item.end());
    return item;
}

// return nth element, based on delimiter
std::string parse(std::string line_data, int element) {

    std::stringstream ss(line_data);
    std::string segment;

    while(std::getline(ss, segment, '^')) {
        if (element == 0) {
           return segment;
        }
        element--;
    }
    return segment;
}

int main(int argc, char *argv[]) {
    (void) argc; (void) *argv;

    // Catch any Xapian::Error exceptions thrown
    try {
        // Make the database
        Xapian::WritableDatabase database(argv[1], Xapian::DB_CREATE_OR_OPEN);

        std::ifstream FOOD_DES(argv[2]);
        std::ifstream NUT_DATA(argv[3]);
        std::string current_food;
        std::string current_nutrient;

        // Read in each line, pull out both the NDB_No and Shrt_Desc
        while (std::getline(FOOD_DES, current_food))
        {
            Xapian::Document document;

            std::stringstream ss(strip(parse(current_food, 2), '~'));
            std::string segment;
            int position = 0;

            while(std::getline(ss, segment, ' ')) {
                if(segment != "") {
                    document.add_posting(strip(segment, ','), position);
                    position++;
                }
            }

            // placeholder to go back to if we have different indexes
            int len = NUT_DATA.tellg();

            std::string index = (parse(current_food, 0));
            std::string dataset = strip(parse(current_food, 2), '~') + " - ";

            // Pull out Nutr_No and Nutr_Val for each matching index
            while (std::getline(NUT_DATA, current_nutrient)) {
                std::string current_index = (parse(current_nutrient, 0));
                if (current_index != index) {
                    NUT_DATA.seekg(len, std::ios_base::beg);
                    break;
                }

                dataset += strip(parse(current_nutrient, 1), '~') + ":";
                dataset += parse(current_nutrient, 2) + " ";

                // update current placeholder
                len = NUT_DATA.tellg();
            }
            document.set_data(dataset);

            // Add the document to the database
            database.add_document(document);
        }
    } catch(const Xapian::Error &error) {
        std::cout << "Exception: " << error.get_msg() << std::endl;
    }
}

