#ifndef URLQueryString_h
#define URLQueryString_h

class URLQueryString
{
public:
    URLQueryString(String query)
    {
        strncpy(fQueryBuffer, query.c_str(), sizeof(fQueryBuffer)-1);
        fNumParams = parse(fQueryBuffer, '&', fParams, SizeOfArray(fParams));
    }

    bool getOptional(const char* id, String& value)
    {
        return get(id, value, false);
    }

    bool get(const char* id, String& value, bool expected = true)
    {
        for (int i = 0; i < fNumParams; i++)
        {
            if (strcmp(id, fParams[i].key) == 0)
            {
                value = String(fParams[i].val);
                return true;
            }
        }
        if (expected)
        {
            DEBUG_PRINT("QUERY ARG NOT FOUND: \""); DEBUG_PRINT(id); DEBUG_PRINTLN("\"");
        }
        return false;
    }

private:
    struct Param
    {
        char* key;
        char* val;
    };

    char fQueryBuffer[1024];
    Param fParams[10];
    int fNumParams;

    static void unescape(char* str)
    {
        char* e;
        while ((e = strchr(str, '%')) != NULL)
        {
            unsigned i = 0;
            uint8_t ch = 0;
            uint8_t ec = e[1];
            if ((ec >= '0' && ec <= '9')) {
                ch = ec - '0'; i++;
            } else if (ec >= 'a' && ec <= 'f') {
                ch = ec - 'a' + 10; i++;
            } else if (ec >= 'A' && ec <= 'F') {
                ch = ec - 'a' + 10; i++;
            }
            ec = e[2];
            if ((ec >= '0' && ec <= '9')) {
                ch = (ch << 4) | (ec - '0'); i++;
            } else if (ec >= 'a' && ec <= 'f') {
                ch = (ch << 4) | (ec - 'a' + 10); i++;
            }
            else if (ec >= 'A' && ec <= 'F') {
                ch = (ch << 4) | (ec - 'A' + 10); i++;
            }
            if (i != 0)
            {
                e[0] = ch;
                memmove(&e[1], &e[3], strlen(&e[3])+1);
            }
        }
        while ((e = strchr(str, '+')) != NULL)
        {
            *e = ' ';
        }
    }

    static int parse(char *query, char delimiter, Param* params, int max_params)
    {
        int i = 0;

        if (NULL == query || '\0' == *query) {
            return -1;
        }

        params[i++].key = query;
        while (i < max_params && NULL != (query = strchr(query, delimiter)))
        {
            *query = '\0';
            params[i].key = ++query;
            params[i].val = NULL;

            /* Go back and split previous param */
            if (i > 0)
            {
                if ((params[i - 1].val = strchr(params[i - 1].key, '=')) != NULL)
                {
                    *(params[i - 1].val)++ = '\0';
                    unescape(params[i - 1].val);
                }
            }
            i++;
        }

        /* Go back and split last param */
        if ((params[i - 1].val = strchr(params[i - 1].key, '=')) != NULL)
        {
            *(params[i - 1].val)++ = '\0';
            unescape(params[i - 1].val);
        }
        return i;
    }
};
#endif
