#ifndef StringUtils_h_
#define StringUtils_h_

int atoi(const char* cmd, int numdigits)
{
    int result = 0;
    for (int i = 0; i < numdigits; i++)
        result = result*10 + (cmd[i]-'0');
    return result;
}

int32_t strtol(const char* cmd, const char** endptr)
{
    bool sign = false;
    int32_t result = 0;
    if (*cmd == '-')
    {
        cmd++;
        sign = true;
    }
    while (isdigit(*cmd))
    {
        result = result*10L + (*cmd-'0');
        cmd++;
    }
    *endptr = cmd;
    return (sign) ? -result : result;
}

uint32_t strtolu(const char* cmd, const char** endptr)
{
    uint32_t result = 0;
    while (isdigit(*cmd))
    {
        result = result*10L + (*cmd-'0');
        cmd++;
    }
    *endptr = cmd;
    return result;
}

bool startswith(const char* &cmd, const char* str)
{
    size_t len = strlen(str);
    if (strncmp(cmd, str, len) == 0)
    {
        cmd += len;
        return true;
    }
    return false;
}

bool startswith_P(const char* &cmd, PROGMEMString str)
{
    size_t len = strlen_P((const char*)str);
    if (strncmp_P(cmd, (const char*)str, len) == 0)
    {
        cmd += len;
        return true;
    }
    return false;
}
#endif
