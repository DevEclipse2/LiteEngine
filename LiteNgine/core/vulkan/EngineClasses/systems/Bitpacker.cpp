mespace lte
{
    char packBools(const bool bools[8]) {
        char packed = 0;
        for (int i = 0; i < 8; ++i) {
            if (bools[i]) {
                packed |= (1 << i);
            }
        }
        return packed;
    }
    void unpackChar(char packed, bool outBools[8]) {
        for (int i = 0; i < 8; ++i) {
            outBools[i] = (packed >> i) & 1;
        }
    }
}