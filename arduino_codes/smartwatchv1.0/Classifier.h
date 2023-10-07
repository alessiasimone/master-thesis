#pragma once
#include <cstdarg>
namespace Eloquent {
    namespace ML {
        namespace Port {
            class LogisticRegression {
                public:
                    /**
                    * Predict class for features vector
                    */
                    int predict(float *x) {
                        float votes[2] = { 3.402985415439  };
                        votes[0] += dot(x,   -1.811803838482  , 0.704520960418  , -1.68340605674  , 0.103833722597  , -3.12536093722  , 0.0  , -1.412278207014  , 19.665805091945  , -35.246074183757  , -25.897337754757  , -35.370120072377  , -26.496047691619  , 92.420450994274  , 33.815407949674  , -131.759056705731  , 68.840406943896  , 0.214028398722 );
                        // return argmax of votes
                        uint8_t classIdx = 0;
                        float maxVotes = votes[0];

                        for (uint8_t i = 1; i < 2; i++) {
                            if (votes[i] > maxVotes) {
                                classIdx = i;
                                maxVotes = votes[i];
                            }
                        }

                        return classIdx;
                    }

                    /**
                    * Predict readable class name
                    */
                    const char* predictLabel(float *x) {
                        return idxToLabel(predict(x));
                    }

                    /**
                    * Convert class idx to readable name
                    */
                    const char* idxToLabel(uint8_t classIdx) {
                        switch (classIdx) {
                            case 0:
                            return "Baseline";
                            case 1:
                            return "Stress";
                            default:
                            return "Houston we have a problem";
                        }
                    }

                protected:
                    /**
                    * Compute dot product
                    */
                    float dot(float *x, ...) {
                        va_list w;
                        va_start(w, 17);
                        float dot = 0.0;

                        for (uint16_t i = 0; i < 17; i++) {
                            const float wi = va_arg(w, double);
                            dot += x[i] * wi;
                        }

                        return dot;
                    }
                };
            }
        }
    }