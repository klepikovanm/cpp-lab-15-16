#include <iostream>
#include <fstream> 
#include <string> 
#include <vector>
#include <thread> //для потоков
#include <future> 
#include <chrono>
using namespace std;

template <typename T>
class Matrix{  
private: 
    int lines, columns;
    T** matrix;

    //Удаление матрицы
    void clear(){
        if (matrix != nullptr) {
            for (int i=0; i<lines; i++) {
                delete[] matrix[i];
            }
        delete[] matrix;
        }
    }

    //Функция для нахождения дополнений
    double addition(int stroka, int stolb) {
        Matrix temp(lines-1,columns-1); 
        int m_l,m_c;
        m_l=0;
        for (int t_l=0; t_l<lines-1; t_l++) {
            if (t_l==stroka-1) {
                m_l=1;
            }
            m_c=0;
            for (int t_c=0; t_c<lines-1; t_c++) {
                if (t_c == stolb-1) {
                    m_c = 1;
                }
                temp.matrix[t_l][t_c] = matrix[t_l+m_l][t_c+m_c];
            }
        }
        int one;
        if ((stroka+stolb)%2==0) {
            one = 1;
        } else {
            one = -1;
        }
        return one*temp.determinant();
    }
    //функция для вычисления размера блоков
    int BlockSize(int l, int c) {
        for (int i=100; i>1; i--) {
            if (l % i == 0 && c % i == 0) {
                return i;
            }
        }
        return 1;
    }
public: 
    //Конструктор по умолчанию(пустая матрица)
    Matrix() {
        lines = 0;
        columns = 0;
        matrix = nullptr;
    } 
    //Конструктор матрицы с заданными размерами
    Matrix(int k_lines, int k_columns) {
        lines = k_lines;
        columns = k_columns;
        matrix = new T* [lines]; //выделение динамической памяти под массив указателей
        for (int i=0; i<lines; i++) {
             matrix[i] = new T [columns]; //выделение динамической памяти для массива значений
        }
    }
    //Конструктор для считывания матрицы из файла
    Matrix(ifstream& in) {
        in >> lines;
        in >> columns;
        matrix = new T* [lines];
        for (int i=0; i<lines; i++) {
            matrix[i] = new T [columns];
            for (int j=0; j<columns; j++) {
                in >> matrix[i][j];
            }
        }
    }
    //Конструктор копирования
    Matrix(const Matrix & M) {
        lines = M.lines;
        columns = M.columns;
        matrix = new T* [lines];
        T** mx = matrix;
        for (int i=0; i<lines; i++) {
             matrix[i] = new T [columns];
        }
        for (int i=0; i<lines; i++){
            for (int j=0; j<columns; j++) {
                mx[i][j]=M.matrix[i][j];
            }
        }

    }
    //Деструктор
    ~Matrix() {
        clear();
    }
    //Получение вне класса доступ к параметрам матрицы
    int get_lines() const {
        return lines;
    }
    int get_columns() const {
        return columns;
    }
    Matrix get_matrix() const {
        return matrix;
    }

    //Ввод с консоли >>
    friend istream& operator>>(istream& in, Matrix& M) {
        cout << "Ввод матрицы" << endl;
        if (M.lines == 0 || M.columns == 0) {
            cout << "Введите количество строк: ";
            in >> M.lines;
            cout << "Введите количество столбцов: ";
            in >> M.columns;
        }
        M.matrix = new T* [M.lines];
        for (int i=0; i<M.lines; i++) {
            M.matrix[i] = new T [M.columns];
            for (int j=0; j<M.columns; j++) {
                cout << "Введите число: строка - " << i+1 << "; столбец - " << j+1 << ": ";
                in >> M.matrix[i][j];
            }
        }
        cout << endl;
        return in;
    }
    //Вывод в консоль <<
    friend ostream& operator<<(ostream& out, const Matrix& M) {
        for (int i=0; i<M.lines; i++) {
            for (int j=0; j<M.columns; j++) {
                out << M.matrix[i][j] << " ";
            }
            out << endl;
        }
        out << endl;
        return out;
    }
    //Ввод с файла >>
    friend ifstream& operator>>(ifstream& infile, Matrix& M) {
        infile >> M.lines;
        infile >> M.columns;
        M.matrix = new T* [M.lines];
        for (int i=0; i<M.lines; i++) {
            M.matrix[i] = new T [M.columns];
            for (int j=0; j<M.columns; j++) {
                infile >> M.matrix[i][j];
            }
        }
        return infile;
    }
    //Вывод в файл <<
    friend ofstream& operator<<(ofstream& outfile, const Matrix& M) {
        for (int i=0; i<M.lines; i++) {
            for (int j=0; j<M.columns; j++) {
                outfile << M.matrix[i][j] << " ";
            }
            outfile << endl;
        }
        return outfile;
    }

    //перегрузка операторов + и -
    Matrix operator +(const Matrix & second) {
        if (this->lines == second.lines && this->columns == second.columns) {
            auto start = chrono::high_resolution_clock::now();//позволяет получить текущее время
            Matrix result(lines,columns);
            vector<thread> threads;
            int num_block_lines = 2; //для 4 потоков делим на 4 блока
            int num_block_columns = 2;
            int block_size = lines / 2;
            for (int i=0; i<num_block_lines; i++) {
                for (int j=0; j<num_block_columns; j++) {
                    threads.emplace_back([this, &second, i, j, block_size, &result] { 
                        for (int l=0; l<block_size; l++) {
                            for (int c=0; c<block_size; c++) {
                                result.matrix[i * block_size + l][j * block_size + c] = this->matrix[i * block_size + l][j * block_size + c] + second.matrix[i * block_size + l][j * block_size + c]; //пусть блоки нумеруются от нуля, тогда по индексам i j мы можем определить в каком блоке находимся, а прибавляя к произведению l или c, мы определяем элемент
                            }
                        }
                    });
                }
            }
            for (int i = 0; i < threads.size(); i++) { 
                threads[i].join();
            }
            auto end = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::nanoseconds>(end - start);
            cout << "Время сложения матриц " << lines << "x" << columns << " - " << duration.count() << " наносекунд" << endl;
            return result;
        } 
        throw "Матрицы разного порядка, сложить нельзя!"; 
    }
    Matrix operator -(const Matrix & second) {
        if (this->lines == second.lines && this->columns == second.columns) {
            vector<Matrix> results;
            vector<int> block_sizes_lines = {500, 250, 250, 125};//потоки 1 - 500x500, 2 - 250x500, 4 - 250x250, 8 - 125x250
            vector<int> block_sizes_columns = {500, 500, 250, 125};
            int block_size_lines;
            int block_size_columns;
            int num_thread;
            for (int i=0; i<4; i++) {
                auto start = chrono::high_resolution_clock::now();
                block_size_lines = block_sizes_lines[i];
                block_size_columns = block_sizes_columns[i];
                if (i == 0) {
                    num_thread = 1;
                } else if (i == 1) {
                    num_thread = 2;
                } else if (i == 2) {
                    num_thread = 4;
                } else if (i == 3) {
                    num_thread = 8;
                }
                Matrix result(lines,columns);
                vector<thread> threads;
                int num_block_lines = lines / block_size_lines;
                int num_block_columns = columns / block_size_columns;
                for (int i=0; i<num_block_lines; i++) {
                    for (int j=0; j<num_block_columns; j++) {
                        threads.emplace_back([this, &second, i, j, block_size_lines, block_size_columns, &result] {
                            for (int l=0; l<block_size_lines; l++) {
                                for (int c=0; c<block_size_columns; c++) {
                                    result.matrix[i * block_size_lines + l][j * block_size_columns + c] = this->matrix[i * block_size_lines + l][j * block_size_columns + c] - second.matrix[i * block_size_lines + l][j * block_size_columns + c]; 
                                }
                            }
                        });
                    }
                }
                for (int i = 0; i < threads.size(); i++) {
                    threads[i].join();
                }
                auto end = chrono::high_resolution_clock::now();
                auto duration = chrono::duration_cast<chrono::nanoseconds>(end - start);
                cout << "Количество потоков - " << num_thread << " Время вычитания матриц - " << duration.count() << " наносекунд" << endl;
                results.push_back(result);
            }
            return results[3];
        } 
        throw "Матрицы разного порядка, вычесть нельзя!";
    }

    //перегрузка оператора * для умножения матриц и умножения матрицы на скаляр
    Matrix operator *(const Matrix & second) {
        if (this->columns == second.lines) {
            cout << "Матрица произведения:" << endl;
            Matrix result(lines,second.columns);
            vector<thread> threads;
            for (int i=0; i<lines; i++) {//потоков будет столько, сколько строк в первой матрице
                threads.emplace_back([this, &second, i, &result] {
                    for (int j=0; j<second.columns; j++) {
                        result.matrix[i][j] = 0;
                        for (int k=0; k<columns; k++) {
                            result.matrix[i][j] += this->matrix[i][k] * second.matrix[k][j];
                        }
                    }
                });  
            }
            for (int i = 0; i < threads.size(); i++) {
                threads[i].join();
            }
            return result;
        }
        throw "Кол-во столбцов 1-й матрицы не равно кол-ву строк 2-й матрицы, умножить нельзя!";
    } 
    Matrix operator *(T scalar) {
        Matrix result(lines,columns);
        vector<thread> threads;
        int block_size = BlockSize(lines, columns);
        int num_block_lines = lines / block_size;
        int num_block_columns = columns / block_size;
        for (int i=0; i<num_block_lines; i++) {
            for (int j=0; j<num_block_columns; j++) {
                threads.emplace_back([this, &scalar, i, j, block_size, &result] {
                    for (int l=0; l<block_size; l++) {
                        for (int c=0; c<block_size; c++) {
                            result.matrix[i * block_size + l][j * block_size + c] = this->matrix[i * block_size + l][j * block_size + c] * scalar; 
                        }
                    }
                });
            }
        }
        for (int i = 0; i < threads.size(); i++) {
            threads[i].join();
        }
        return result;
    }

    //Функция для нахождения детерминанта
    double determinant() {
        double det=0;
        int one;
        T ** M = this->matrix;
        if (this->lines != this->columns) {
           throw "Матрица не квадратная, нет детерминанта!";
        } else {
            if (lines == 1) { 
                return matrix[0][0];
            } else if (lines == 2) { 
                return (matrix[0][0]*matrix[1][1] - matrix[0][1]*matrix[1][0]);
            } else {
                det = 0;
                for (int i=0; i<lines; i++) {
                    Matrix temp(lines-1,columns-1);
                    for (int j=1; j<lines; j++) {
                        int t = 0;
                        for (int k=0; k<lines; k++) {
                            if (k == i) {
                                continue;
                            }
                            temp.matrix[j-1][t] = matrix[j][k];
                            t++;
                        }
                    }
                    if ((i+1+1)%2==0) {
                        one = 1;
                    } else {
                        one = -1;
                    }
                    det += one*matrix[0][i]*temp.determinant();
                }
            }
        }
        return det;
    }
    //Функция для нахождения присоединенной матрицы - A*
    Matrix join() {
        Matrix result(lines,columns);
        if (lines != columns) {
            throw "Матрица не квадратная, нет присоединенной!";
        } else {
            vector<future<void>> futures;
            for (int i=0; i<lines; i++) {
                futures.push_back(async(launch::async, [i, this, &result]() {
                    for (int j=0; j<columns; j++) {
                        result.matrix[i][j] = addition(i+1,j+1);
                    }
                }));
            }
            for (int i = 0; i < futures.size(); i++) {
                futures[i].get(); 
            }
        } 
        return result;
    }
    //Функция для транспонирования матрицы
    void transposed() {
        T** result = new T*[columns];
        for (int i=0; i<columns; i++) {
            result[i] = new T[lines];
        }
        vector<future<void>> futures;
        for (int i=0; i<columns; i++){
            futures.push_back(async(launch::async, [i, this, &result]() {
                for (int j=0; j<lines; j++) {
                    result[i][j] = matrix[j][i];
                }
            }));
        }
        for (int i = 0; i < futures.size(); i++) {
            futures[i].get(); 
        }
        clear();
        int l = lines;
        lines = columns;
        columns = l;
        matrix = result;
    }
    //Перегрузка оператра ! для вычисления обраной матрицы
    Matrix operator !() {
        Matrix temp(lines,columns);
        for (int i=0; i<lines; i++) {
            for (int j=0; j<lines; j++) {
                temp.matrix[i][j] = matrix[i][j];
            }
        }
        if (lines != columns) {
            throw "Матрица не квадратная, обратной нет!";
        } else {
            double det = temp.determinant();
            if (det == 0) {
                throw "Нулевой детерминант, обратной матрицы нет!";
            } else {
                Matrix temp_2 = temp.join();
                temp_2.transposed();
                Matrix temp_3 = temp_2 * (1/det);
                cout << "Обратная матрица:" << endl;
                return temp_3;
            }
        } 
    }

    Matrix& operator =(const Matrix & second) {
        if (this == &second) {
            return *this;
        }
        clear();
        lines = second.lines;
        columns=second.columns;
        matrix = new T * [lines];
        for (int i=0; i<lines; i++) {
            matrix[i] = new T [columns];
            for (int j=0; j<columns; j++) {
                matrix[i][j] = second.matrix[i][j];
            }
        }
        return *this;
    }

    //Cтатические методы создания нулевых и единичных матриц указанного размера
    static Matrix<T> zero(int k_lines, int k_columns) {
        Matrix<T> result(k_lines, k_columns);
        vector<future<void>> futures;//вектор, который будет хранить в себе результат каждого блока(функция ничего не возвращает, поэтому void)
        int block_size = result.BlockSize(k_lines, k_columns);
        int num_block_lines = k_lines / block_size;
        int num_block_columns = k_columns / block_size;
        for (int i = 0; i < num_block_lines; i++) {
            for (int j = 0; j < num_block_columns; j++) {
                futures.push_back(async(launch::async, [i, j, block_size, &result]() {//Аргумент launch::async указывает, что поток должен быть запущен сразу, а не отложен на потом.
                for (int l=0; l<block_size; l++) {
                        for (int c=0; c<block_size; c++) {
                            result.matrix[i * block_size + l][j * block_size + c] = 0; 
                        }
                    }
                }));
            }
        }
        for (int i = 0; i < futures.size(); i++) {
            futures[i].get();//Ждем завершения всех фьючерсов
        }
        return result;
    }
    static Matrix<T> id(int k_lines, int k_columns) {
        if (k_lines == k_columns) {
            Matrix<T> result(k_lines, k_columns);
            vector<future<void>> futures;
            int block_size = result.BlockSize(k_lines, k_columns);
            int num_block_lines = k_lines / block_size;
            int num_block_columns = k_columns / block_size;
            for (int i = 0; i < num_block_lines; i++) {
                for (int j = 0; j < num_block_columns; j++) {
                    futures.push_back(async(launch::async, [i, j, block_size, &result]() {
                        for (int l=0; l<block_size; l++) {
                            for (int c=0; c<block_size; c++) {
                                if (i!=j) {
                                zero(block_size, block_size);
                                } else {
                                    if ((i * block_size + l)!=(j * block_size + c)) {
                                    result.matrix[i * block_size + l][j * block_size + c] = 0;
                                    } else {
                                    result.matrix[i * block_size + l][j * block_size + c] = 1;
                                    } 
                                }
                            } 
                        }
                    }));
                }
            }
        for (int i = 0; i < futures.size(); i++) {
            futures[i].get();
        }
            return result;
        }
        throw "Матрица не квадратная, она не может быть единичной!";
    }
     
    //Перегрузка операторов == и != для сравнения матриц
    bool operator ==(const Matrix & second){
        if (this->lines == second.lines && this->columns == second.columns) {
            vector<future<bool>> futures;
            int block_size = BlockSize(lines, columns);
            int num_block_lines = lines / block_size;
            int num_block_columns = columns / block_size;
            for (int i = 0; i < num_block_lines; i++) {
                for (int j = 0; j < num_block_columns; j++) {
                    futures.push_back(async(launch::async, [i, j, block_size, &second, this]() {
                        for (int l=0; l<block_size; l++) {
                            for (int c=0; c<block_size; c++) {
                                if (this->matrix[i * block_size + l][j * block_size + c] != second.matrix[i * block_size + l][j * block_size + c]) {
                                    return false;
                                }  
                            }
                        }
                        return true;
                    }));
                }
            }
            for (int i = 0; i < futures.size(); i++) {
                if (futures[i].get() == false) {
                    cout << "Матрицы не равны" << endl;
                    return false;
                }
            }
            cout << "Матрицы равны" << endl;
            return true;

        } else {
            cout << "Матрицы не равны" << endl;
            return false;
        }
    }
    bool operator !=(const Matrix & second){
        return !(*this == second);
    }
    //Перегрузка операторов == и != для сравнения матрицы и скаляра 
    bool operator ==(double scalar) {
        vector<future<bool>> futures;
        int block_size = BlockSize(lines, columns);
        int num_block_lines = lines / block_size;
        int num_block_columns = columns / block_size;
        for (int i = 0; i < num_block_lines; i++) {
            for (int j = 0; j < num_block_columns; j++) {
                futures.push_back(async(launch::async, [i, j, block_size, &scalar, this]() {
                    for (int l=0; l<block_size; l++) {
                        for (int c=0; c<block_size; c++) {
                            if (((i * block_size + l)!=(j * block_size + c)) && (this->matrix[i * block_size + l][j * block_size + c] != 0)) {
                                return false;
                            }  
                            if (((i * block_size + l)==(j * block_size + c)) && (this->matrix[i * block_size + l][j * block_size + c] != scalar)) {
                                return false;
                            } 
                        }
                    }
                    return true;
                }));
            }
        }
        for (int i = 0; i < futures.size(); i++) {
            if (futures[i].get() == false) {
                cout << "Матрица не равна скаляру" << endl;
                return false;
            }
        }
        cout << "Матрица равна скаляру" << endl;
        return true;
    }
    bool operator !=(double scalar) {
        return !(*this == scalar);
    }

    //Функции для сложения и вычитания строк или столбцов (если thing = 0 - строка, если thing = 1 - столбец)
    void plus(int thing, int one, int two, double L) {
        if (thing == 0) {
            for (int i=0; i<columns; i++) {
                matrix[one-1][i] += matrix[two-1][i] * L;
            }
        } else {
            for (int i=0; i<lines; i++) {
                matrix[i][one-1] += matrix[i][two-1] * L;
            }
        }
    }    
    void minus(int thing, int one, int two, double L) {
        if (thing == 0) {
            for (int i=0; i<columns; i++) {
                matrix[one-1][i] -= matrix[two-1][i] * L;
            }
        } else {
            for (int i=0; i<lines; i++) {
                matrix[i][one-1] -= matrix[i][two-1] * L;
            }
        }
    }
    //Функция для преобразования отдельной строки или столбца
    void multiply(int thing, int one, double L) {
        if (thing == 0) {
            for (int i=0; i<columns; i++) {
                matrix[one-1][i] *= L;
            }
        } else {
            for (int i=0; i<lines; i++) {
                matrix[i][one-1] *= L;
            }
        }
    }
    //Функция для перестановки строки или столбца
    void change(int thing, int one, int two) {
       int temp;
       if (thing == 0) {
            for (int i=0; i<columns; i++) {
                temp = matrix[one-1][i];
                matrix[one-1][i] = matrix[two-1][i];
                matrix[two-1][i] = temp;
            }
        } else {
            for (int i=0; i<lines; i++) {
                temp = matrix[i][one-1];
                matrix[i][one-1] = matrix[i][two-1];
                matrix[i][two-1] = temp;
            }
        } 
    }
};

int main() {
    int num_cores = thread::hardware_concurrency(); //количество всех ядер на компьютере=количество потоков, запускаемых одновременно
    cout << '\v' << "Количество ядер: " << num_cores << endl << '\v';

    cout << "№16-1 Измерение времени сложения при разных размерах матрицы и 4-х потоках" << endl;
    Matrix<double> A1, A2, A3, A4, A5, B1, B2, B3, B4, B5, C1, C2, C3, C4, C5, T1, T2, T3;
    ifstream data_file("DataFile16.txt");
    data_file >> A1 >> B1 >> A2 >> B2 >> A3 >> B3 >> A4 >> B4 >> A5 >> B5 >> T1 >> T2;
    data_file.close();

    try {
        C1 = A1 + B1;
    } catch (const char* error_message){
        cout << error_message << endl;
    }
    try {
        C2 = A2 + B2;
    } catch (const char* error_message){
        cout << error_message << endl;
    }
    try {
        C3 = A3 + B3;
    } catch (const char* error_message){
        cout << error_message << endl;
    }
    try {
        C4 = A4 + B4;
    } catch (const char* error_message){
        cout << error_message << endl;
    }
    try {
        C5 = A5 + B5;
    } catch (const char* error_message){
        cout << error_message << endl;
    }

    
    cout << '\v' << "№16-2 Измерение времени вычитания при разном количестве потоков и размере матрицы 500х500" << endl;;
    try {
        T3 = T1-T2;
    } catch (const char* error_message){
        cout << error_message << endl;
    }
    cout << '\v';
}