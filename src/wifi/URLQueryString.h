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
                }
            }
            i++;
        }

        /* Go back and split last param */
        if ((params[i - 1].val = strchr(params[i - 1].key, '=')) != NULL)
        {
            *(params[i - 1].val)++ = '\0';
        }
        return i;
    }
};
#endif
