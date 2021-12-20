#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>


int countWords(std:: string str)
{
    int count = 0, i;
    for (i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == ' ')
            count++;
    }
    return count+1;
}
int countLetters(std:: string str)
{
    int count = 0, i;
    for (i = 0; str[i] != '\0'; i++)
    {
        try {
            if ((str[i] >=65 && str[i]<=90)||(str[i] >= 97 && str[i] <= 122))
                count++;
        }
        catch (const std::exception& e) {
            std::cerr << e.what();
        }
    }
    return count;
}
//Returns vector with bounds of elements divided between threads
std::vector<int> getCompartments(int nr_threads, int nr_elements)
{
    int last_val = 0; //Stores last element of vector 
    int temp_modulo = nr_elements % nr_threads; //Additional part of dividing elements by threads which will be added to last bound
    int temp_divider = nr_elements / nr_threads; //# of elements stored between Left-Right bound
    std::vector<int> bounds_vec{ 0 }; //Stores calculated bounds, starting with '0'
    for (int i = 0; i < nr_threads; i++)
    {
        //For every loop, it emplaces back sum of constant 
        //elements number in every compartment and last emplaced value
        bounds_vec.emplace_back(temp_divider+last_val);
        if (i == nr_threads - 1) bounds_vec[i+1] += temp_modulo; //If it is last bound, add rest of division
        last_val = bounds_vec[i+1]; //Store last emplaced value (i+1, because we have alreadt 0 in vector)
    }
    return bounds_vec;
}
//Count lines in files from given vector with paths, save result to given reference variable
void countSomeFiles(const std::vector<std::filesystem::path> &paths_vector, std::atomic_int&lines_counter, std::atomic_int&words_counter, std::atomic_int&letters_counter, int left_bound, int right_bound, int no)
{
    
    for (int i = left_bound; i < right_bound; i++)
    {   
        std::string temps;
        try {
            //Open file
            std::ifstream inFile(paths_vector[i]);
            lines_counter += std::count(std::istreambuf_iterator<char>(inFile), std::istreambuf_iterator<char>(), '\n') + 1;
            inFile.close();
            inFile.open(paths_vector[i]);
            if (inFile.is_open())
            {
                while (inFile >> temps)
                {
                    words_counter += countWords(temps);
                    letters_counter += countLetters(temps);
                }
                inFile.close();
            }
        }
        catch (const std::exception& e) {
            std::cerr << e.what();
        }
    }
}
bool is_number(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(),
        s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

int main()
{
    int available_threads = std::thread::hardware_concurrency(); //Check how many threads is available
    std::atomic_int line_counter = 0;
    std::atomic_int words_counter = 0;
    std::atomic_int letters_counter = 0;
    bool is_path_exist = false;
    bool is_no_threads_available = false;
    std::string scan_path = "";
    std::string choosen_threads_number = "";
    std::vector<std::filesystem::path> files_paths;
    std::vector<std::thread> threads;

    //Check if given directorcy exists
    printf("Welcome to the directory analyzer, enter directory path to analyze: ");
    do {
        std::getline(std::cin, scan_path);
        if (!(is_path_exist = std::filesystem::exists(scan_path)))
        {
            system("cls");
            printf("Given path does not exist, please enter once again: ");
        }
    } while (!is_path_exist);

    std::filesystem::path p1{ scan_path };

    //Chceck if given no threads is available
    printf("\nThere are %d available threads, how much would u use?: ", available_threads);
    do {
        std::getline(std::cin, choosen_threads_number);
        if (is_no_threads_available = is_number(choosen_threads_number))
        {
            if (is_no_threads_available = (stoi(choosen_threads_number) <= available_threads && stoi(choosen_threads_number) > 0))
            {
                available_threads = stoi(choosen_threads_number);
            }
            else
            {
                system("cls");
                printf("Given number of threads is not available, please choose 1-%d: ", available_threads);
            }
        }
        else
        {
            system("cls");
            printf("Given input is not a number! Please choose 1-%d: ", available_threads);
        }
    } while (!is_no_threads_available);
    
    //Add every file path in given recursive directory to paths vector
    for (auto& p : std::filesystem::recursive_directory_iterator(p1, std::filesystem::directory_options::skip_permission_denied))
    {
        files_paths.emplace_back(p.path());
    }
    
    //Divide vector of all paths to compartments and save to vector
    std::vector<int> compartments = getCompartments(available_threads, files_paths.size());

    //Start measuring operation time
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < available_threads; i++)
    {
        threads.emplace_back(std::thread(countSomeFiles, std::ref(files_paths), std::ref(line_counter),std::ref(words_counter), std::ref(letters_counter), compartments[i], compartments[i+1], i+1));
    }
    for (auto &t: threads)
    {
        t.join();
    }
    //Stop timer
    auto finish = std::chrono::high_resolution_clock::now();
    //Calculate delta time
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Execution time with " << available_threads << " used: " << elapsed.count()<<" seconds\n";
    std::cout << "# of files in " << p1 << ": " << files_paths.size() << '\n';
    std::cout << "# of lines in all files: " << line_counter << '\n';
    std::cout << "# of words in all files: " << words_counter << '\n';
    std::cout << "# of letters in all files: " << letters_counter << '\n';


}