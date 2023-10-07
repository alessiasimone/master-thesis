#ifndef UUID2673475314272
#define UUID2673475314272

#include <cstring>


namespace extractor {

    
        #ifndef UUID2673475313312
#define UUID2673475313312

/**
  * Window(length=60, shift=1)
 */
class Step0 {
    public:

        /**
         * Transform input vector
         */
        bool transform(float *x) {
            
    // append x to queue
    memcpy(queue + head, x, sizeof(float) * 2);
    head += 2;

    if (head != 120)
        return false;

    // copy queue to x and shift
    memcpy(x, queue, sizeof(float) * 120);

    
        memcpy(queue, queue + 2, sizeof(float) * 118);
    

    head -= 2;



            return true;
        }

    protected:
        
    uint16_t head = 0;
    float queue[120] = {0};

};



#endif
    
        #ifndef UUID2673475314512
#define UUID2673475314512

/**
  * SpectralFeatures(num_inputs=2, window_length=60, order=1, num_features=8)
 */
class Step1 {
    public:

        /**
         * Transform input vector
         */
        bool transform(float *x) {
            
    // spectral features
    uint16_t idx = 0;

    for (uint16_t k = 0; k < 2; k++) {
        float minimum = 9999999;
        float maximum = -9999999;
        float abs_minimum = 999999;
        float abs_maximum = 0;
        float mean = 0;
        float abs_energy = 0;
        float mean_abs_change = 0;
        float cid_ce = 0;
        float xi_prev = x[k];

        // first-order features
        for (uint16_t i = k; i < 120; i += 2) {
            float xi = x[i];
            float abs_xi = abs(xi);
            float xi_prev2 = (i >= k + 4) ? x[i - 4] : xi;

            float xi2 = xi * xi;
            float diff = xi - xi_prev;

            // start of features
            minimum = xi < minimum ? xi : minimum;
            maximum = xi > maximum ? xi : maximum;
            abs_minimum = abs_xi < abs_minimum ? abs_xi : abs_minimum;
            abs_maximum = abs_xi > abs_maximum ? abs_xi : abs_maximum;
            mean += xi;
            abs_energy += xi2;
            mean_abs_change += abs(diff);
            cid_ce += diff * diff;
            xi_prev = xi;
        }

        mean /= 60;
        abs_energy /= 60;
        mean_abs_change /= 60;
        cid_ce /= 60;

        buffer[idx++] = maximum;
        buffer[idx++] = minimum;
        buffer[idx++] = abs_maximum;
        buffer[idx++] = abs_minimum;
        buffer[idx++] = mean;
        buffer[idx++] = abs_energy;
        buffer[idx++] = mean_abs_change;
        buffer[idx++] = cid_ce;

        
    }

    memcpy(x, buffer, 16 * sizeof(float));


            return true;
        }

    protected:
        
    float buffer[16];

};



#endif
    
        #ifndef UUID2673475314752
#define UUID2673475314752

/**
  * MinMaxScaler(low=0, high=1)
 */
class Step2 {
    public:

        /**
         * Transform input vector
         */
        bool transform(float *x) {
            
    for (uint16_t i = 0; i < 16; i++) {
        x[i] = (x[i] - offset[i]) * scale[i] + 0;

        if (x[i] < 0) x[i] = 0;
        else if (x[i] > 1) x[i] = 1;
    }

    return true;


            return true;
        }

    protected:
        
    float offset[16] = {52.78333333333f, 52.15000000000f, 52.78333333333f, 52.15000000000f, 52.32783333333f, 2738.22110981482f, 0.02005649718f, 0.00071864407f, 28.29069044367f, 28.17136952712f, 28.29069044367f, 28.17136952712f, 28.23352537410f, 797.13317183750f, 0.00336552582f, 0.00001989303f};
    float scale[16] = {0.01662602527f, 0.01911436763f, 0.01662602527f, 0.01911436763f, 0.01841718610f, 0.00011584217f, 1.09326744904f, 0.10006280590f, 0.12998628154f, 0.12926526472f, 0.12998628154f, 0.12926526472f, 0.12993008997f, 0.00202498885f, 1.83099304309f, 2.28257942216f};

};



#endif
    

     /**
      * Pipeline:
 * ---------
 *  > Window(length=60, shift=1)
 *  > SpectralFeatures(num_inputs=2, window_length=60, order=1, num_features=8)
 *  > MinMaxScaler(low=0, high=1)
     */
    class Pipeline {
        public:
            static const uint16_t NUM_INPUTS = 2;
            static const uint16_t NUM_OUTPUTS = 16;
            static const uint16_t WORKING_SIZE = 120;
            float X[120];

            /**
             * Apply pipeline to input vector
             */
            template<typename T>
            bool transform(T *x) {
                for (uint16_t i = 0; i < 2; i++)
                    this->X[i] = x[i];

                size_t start = micros();
                bool isOk =
                
                     step0.transform(X)
                
                    && step1.transform(X)
                
                    && step2.transform(X)
                ;

                latency = micros() - start;

                return isOk;
            }

            /**
             * Debug output feature vector
             */
            template<typename PrinterInterface>
            void debugTo(PrinterInterface &printer, uint8_t precision=5) {
                printer.print(X[0], precision);

                for (uint16_t i = 1; i < 16; i++) {
                    printer.print(", ");
                    printer.print(X[i], precision);
                }

                printer.print('\n');
            }

            /**
 * Get latency in micros
 */
uint32_t latencyInMicros() {
    return latency;
}

/**
 * Get latency in millis
 */
uint16_t latencyInMillis() {
    return latency / 1000;
}

        protected:
            float latency;
            
                extractor::Step0 step0;
            
                extractor::Step1 step1;
            
                extractor::Step2 step2;
            
    };
}


static extractor::Pipeline processor;


#endif