#ifndef UUID2027058754992
#define UUID2027058754992

#include <cstring>


namespace scaler {

    
        #ifndef UUID2027058754944
#define UUID2027058754944

/**
  * MinMaxScaler(low=0, high=1)
 */
class Step0 {
    public:

        /**
         * Transform input vector
         */
        bool transform(float *x) {
            
    for (uint16_t i = 0; i < 17; i++) {
        x[i] = (x[i] - offset[i]) * scale[i] + 0;

        if (x[i] < 0) x[i] = 0;
        else if (x[i] > 1) x[i] = 1;
    }

    return true;


            return true;
        }

    protected:
        
    float offset[17] = {52.15000000000f, 52.78333333333f, 52.32783333333f, 52.28333333333f, 0.00002222227f, 0.00471405041f, -6.41909133721f, -2.52696844642f, 28.17136952712f, 28.29069044367f, 28.23352537410f, 28.22770905391f, 0.00000000177f, 0.00004209686f, -7.69748356341f, -4.25852281206f, 0.00000000000f};
    float scale[17] = {0.01911436763f, 0.01662602527f, 0.01841718610f, 0.01823043267f, 0.00616028540f, 0.07851653087f, 0.08944538108f, 0.02042162766f, 0.12926526472f, 0.12998628154f, 0.12993008997f, 0.12985660484f, 0.71374781773f, 0.84486601797f, 0.06517074463f, 0.01568629232f, 0.07142857143f};

};



#endif
    

     /**
      * Pipeline:
 * ---------
 *  > MinMaxScaler(low=0, high=1)
     */
    class Pipeline {
        public:
            static const uint16_t NUM_INPUTS = 17;
            static const uint16_t NUM_OUTPUTS = 17;
            static const uint16_t WORKING_SIZE = 17;
            float X[17];

            /**
             * Apply pipeline to input vector
             */
            template<typename T>
            bool transform(T *x) {
                for (uint16_t i = 0; i < 17; i++)
                    this->X[i] = x[i];

                size_t start = micros();
                bool isOk =
                
                     step0.transform(X)
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

                for (uint16_t i = 1; i < 17; i++) {
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
            
                scaler::Step0 step0;
            
    };
}


static scaler::Pipeline processor;


#endif