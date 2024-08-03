
int main()
{

    //a constant pointer only means that you cant use
    //the pointer to change the value of the int it's pointing to.
    //you can however change it

    //this is somehow different than the expected semantics I would have expected. I know that for simple concret data types
    //like int, you can't modify its value if it had been declared const intiailly. INteresting.

    int i = 42; // non const object
    const int* r1 = &i; // const reference to non const object
    int anotherInt = 5;
    r1 = &anotherInt;


    int j = 25; // non const object
    int &r2 = j; // non const reference to non const object

}
