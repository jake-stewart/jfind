#include <vector>
#include <thread>
#include <functional>

template <class T>
class ThreadManager {
    public:
        ThreadManager(std::function<void(T*, int)> callback) {
            m_nThreads = 4;
            m_threshold = 1;
            m_callback = callback;
        }

        void run(std::vector<T>& data) {
            run(data.data(), data.size());
        }

        void run(T *data, int n_data) {
            if (m_nThreads == 1 || n_data < m_threshold) {
                runSingleThreaded(data, n_data);
            }
            else {
                runMultiThreaded(data, n_data);
            }
        }

        void setNumThreads(int n_threads) {
            m_nThreads = n_threads;
        }

        void setThreshold(int threshold) {
            // number of entries required before multithreading is used
            m_threshold = threshold;
        }

    private:
        int m_nThreads;
        int m_threshold;
        std::function<void(T*, int)> m_callback;

        void runSingleThreaded(T *data, int n_data) {
            m_callback(data, n_data);
        }

        void runMultiThreaded(T *data, int n_data) {
            int portion = n_data / m_nThreads;
            std::vector<std::thread> threads(m_nThreads - 1);

            for (int i = 0; i < m_nThreads - 1; i++) {
                threads[i] = std::thread(
                    m_callback,
                    data + (portion * i),
                    portion
                );
            }

            int remaining = portion + n_data % m_nThreads;
            m_callback(
                data + (n_data - remaining),
                remaining
            );

            for (int i = 0; i < m_nThreads - 1; i++) {
                threads[i].join();
            }
        }
};
