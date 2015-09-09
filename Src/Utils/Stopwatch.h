#pragma once


class Stopwatch 
{
public:
	void Start(){ 
		m_start = clock(); 
	}
	void Stop(){ 
		m_stop = clock(); 		
		m_sum+= (1.0f* (m_stop - m_start) / CLOCKS_PER_SEC);
		}

	void Tick()
	{
		m_stop = clock();
		m_sum += (1.0f* (m_stop - m_start) / CLOCKS_PER_SEC);
		m_start = m_stop;
	}
		 
	void Reset() { m_sum = 0; }
	float Seconds(){ return m_sum; }
private:
	clock_t m_start;
	clock_t  m_stop;
	float m_sum=0;
};
