#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <regex.h>

int download_file(char* url, char* filename) {
    CURL *curl;
    FILE *fp;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        fp = fopen(filename, "wb");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        /* always cleanup */
        curl_easy_cleanup(curl);
        fclose(fp);
	return 0;
    } else {
        return 1;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <file>");
        return 1;
    }
    FILE *file = fopen(argv[1], "r");
    FILE *new_file = fopen("source_file.mod.c", "w");
    char line[256];
    regex_t regex;
    regmatch_t matches[2]; // we need only the first match

    // Compile the regular expression
    regcomp(&regex, "#include \"(http://[^\"]*)\"", REG_EXTENDED);
    char url[256];
    while (fgets(line, sizeof(line), file)) {
        if (regexec(&regex, line, 2, matches, 0) == 0) {
            // matches[1] contains the first match
            strncpy(url, &line[matches[1].rm_so], matches[1].rm_eo - matches[1].rm_so);
            url[matches[1].rm_eo - matches[1].rm_so] = '\0'; // null terminate the url string

            char filename[256];
            strcpy(filename, strrchr(url, '/') + 1);
            strcat(filename, ".h");

            if (download_file(url, filename) != 0) {
                regfree(&regex);
                fclose(file);
                fclose(new_file);
                break;
                return 1;
            }

            // Replace the URL with the local file name
            strncpy(&line[matches[1].rm_so], filename, strlen(filename));
            line[matches[1].rm_so + strlen(filename)] = '\"'; // replace the end quote
            line[matches[1].rm_so + strlen(filename) + 1] = '\n'; // add a newline
            line[matches[1].rm_so + strlen(filename) + 2] = '\0'; // null terminate the line
        }
        fputs(line, new_file);
    }

    // Free the compiled regular expression
    regfree(&regex);

    fclose(file);
    fclose(new_file);

    // Compile the source file
    if (system("gcc -o output source_file.mod.c -Wpedantic --pedantic") != 0) {
        return 1;
    }

    // Remove the downloaded .h files and the source_file.mod.c
    while (fgets(line, sizeof(line), file)) {
        if (regexec(&regex, line, 2, matches, 0) == 0) {
            char filename[256];
            strcpy(filename, strrchr(url, '/') + 1);
            strcat(filename, ".h");
            remove(filename);
        }
    }
    remove("source_file.mod.c");

    return 0;
}
