#include <math.h>

#include "s-basis.h"

/* Takes two vectors and fills C with their convolution. 
template <typename T>
void convolve(std::vector<T> &A, std::vector<T> &B, std::vector<T> &C) {
    C.resize(A.size() + B.size(), BezOrd(0,0));
    fill(C.begin(), C.end(), 0);
    
    for(unsigned j = 0; j < B.size(); j++) {
        for(unsigned i = j; i < A.size()+j; i++) {
            C[i] += B[j]*A[i-j];
        }
    }
    }*/

double SBasis::tail_error(unsigned tail) const {
    double err = 0;
    for(unsigned i = tail; i < size(); i++) {
        err += fabs(a[i][0]) + fabs(a[i][1]);
    }
    return err;
}

SBasis operator*(double k, SBasis const &a) {
    SBasis c;
    c.a.resize(a.size(), BezOrd(0,0));
    for(unsigned j = 0; j < a.size(); j++) {
        for(unsigned dim = 0; dim < 2; dim++)
            c[j][dim] += k*a[j][dim];
    }
    return c;
}

SBasis shift(SBasis const &a, int sh) {
    SBasis c = a;
    if(sh > 0) {
        c.a.insert(c.a.begin(), sh, BezOrd(0,0));
    } else {
        // truncate
    }
    return c;
}

SBasis shift(BezOrd const &a, int sh) {
    SBasis c;
    if(sh > 0) {
        c.a.insert(c.a.begin(), sh, BezOrd(0,0));
        c.a.push_back(a);
    } else {
        // truncate
    }
    return c;
}

SBasis truncate(SBasis const &a, unsigned terms) {
    SBasis c;
    if(terms > a.size())
        terms = a.size();
    c.a.insert(c.a.begin(), a.a.begin(), a.a.begin() + terms);
    return c;
}

SBasis multiply(SBasis const &a, SBasis const &b) {
    // c = {a0*b0 - shift(1, a.Tri*b.Tri), a1*b1 - shift(1, a.Tri*b.Tri)}
    
    // shift(1, a.Tri*b.Tri)
    SBasis c;
    c.a.resize(a.size() + b.size(), BezOrd(0,0));
    c.a[0] = BezOrd(0,0);
    for(unsigned j = 0; j < b.size(); j++) {
        for(unsigned i = j; i < a.size()+j; i++) {
            double tri = Tri(b[j])*Tri(a[i-j]);
            c.a[i+1/*shift*/] += Hat(-tri);
        }
    }
    for(unsigned j = 0; j < b.size(); j++) {
        for(unsigned i = j; i < a.size()+j; i++) {
            for(unsigned dim = 0; dim < 2; dim++)
                c[i][dim] += b[j][dim]*a[i-j][dim];
        }
    }
    c.normalize();
    //assert(!(0 == c.a.back()[0] && 0 == c.a.back()[1]));
    return c;
}

SBasis integral(SBasis const &c) {
    SBasis a;
    a.a.resize(c.size() + 1, BezOrd(0,0));
    a.a[0] = BezOrd(0,0);
    
    for(unsigned k = 1; k < c.size() + 1; k++) {
        double ahat = -Tri(c[k-1])/(2*k);
        a[k] = Hat(ahat);
    }
    double aTri = 0;
    for(int k = c.size()-1; k >= 0; k--) {
        aTri = (Hat(c[k]) + (k+1)*aTri/2)/(2*k+1);
        a[k][0] -= aTri/2;
        a[k][1] += aTri/2;
    }
    a.normalize();
    return a;
}

SBasis derivative(SBasis const &a) {
    SBasis c;
    c.a.resize(a.size(), BezOrd(0,0));
    
    for(unsigned k = 0; k < a.size(); k++) {
        double d = (2*k+1)*Tri(a[k]);
        
        for(unsigned dim = 0; dim < 2; dim++) {
            c[k][dim] = d;
            if(k+1 < a.size()) {
                if(dim)
                    c[k][dim] = d - (k+1)*a[k+1][dim];
                else
                    c[k][dim] = d + (k+1)*a[k+1][dim];
            }
        }
    }
    
    return c;
}

SBasis sqrt(SBasis const &a, int k) {
    SBasis c;
    
    c.a.push_back(BezOrd(sqrt(a[0][0]), sqrt(a[0][1])));
    SBasis r = a - multiply(c, c); // remainder
    
    for(unsigned i = 1; i <= k; i++) {
        BezOrd ci(r[i][0]/(2*c[0][0]), r[i][1]/(2*c[0][1]));
        SBasis cisi = shift(ci, i);
        r = r - multiply(shift((2*c + cisi), i), SBasis(ci));
        c = c + cisi;
        r.normalize();
        if(r.size() == 0) // if exact
            break;
    }
    
    return c;
}

// return a kth order approx to 1/a)
SBasis reciprocal(BezOrd const &a, int k) {
    SBasis res;
    double r_s0 = (Tri(a)*Tri(a))/(-a[0]*a[1]);
    double r_s0k = 1;
    for(int i = 0; i < k; i++) {
        res.a.push_back(BezOrd(r_s0k/a[0], r_s0k/a[1]));
        r_s0k *= r_s0;
    }
    return res;
}

SBasis divide(SBasis const &a, SBasis const &b, int k) {
    SBasis c;
    SBasis r = a; // remainder
    
    k++;
    r.a.resize(k, BezOrd(0,0));
    c.a.resize(k, BezOrd(0,0));

    for(unsigned i = 0; i < k; i++) {
        BezOrd ci(r[i][0]/b[0][0], r[i][1]/b[0][1]); //H0
        c[i] = c[i] + ci;
        //r[i] = r[i] - ci;
        r = r - shift(multiply(ci,b), i);
        r.normalize();
        if(r.size() == 0) // if exact
            break;
    }
    
    return c;
}

// a(b)
// return a0 + s(a1 + s(a2 +...  where s = (1-u)u; ak =(1 - u)a^0_k + ua^1_k
SBasis compose(SBasis const &a, SBasis const &b) {
    SBasis s = multiply((SBasis(BezOrd(1,1))-b), b);
    SBasis r;
    
    for(int i = a.size()-1; i >= 0; i--) {
        r = SBasis(BezOrd(Hat(a[i][0]))) - a[i][0]*b + a[i][1]*b + multiply(r,s);
    }
    return r;
}

/*
Inversion algorithm. The notation is certainly very misleading. The
pseudocode should say:

c(v) := 0
r(u) := r_0(u) := u
for i:=0 to k do
  c_i(v) := H_0(r_i(u)/(t_1)^i; u)
  c(v) := c(v) + c_i(v)*t^i
  r(u) := r(u) ? c_i(u)*(t(u))^i
endfor
*/

SBasis inverse(SBasis const &a, int k) {
    SBasis c;                           // c(v) := 0
    SBasis r = BezOrd(0,1);             // r(u) := r_0(u) := u
    BezOrd t1(1+a[1][0], 1-a[1][1]);    // t_1
    BezOrd one(1,1);
    BezOrd t1i = one;                   // t_1^0
    SBasis t = multiply(SBasis(one) - a, a); // t(u)
    SBasis ti(one);                     // t(u)^0
    std::cout << "a=" << a << std::endl;
    std::cout << "t1=" << t1 << std::endl;

    for(unsigned i = 0; i < k; i++) {   // for i:=0 to k do
        std::cout << i << ": " <<std::endl;
        std::cout << "r=" << r << std::endl
                  << "c=" << c << std::endl
                  << "ti=" << ti << std::endl
                  << std::endl;
        if(r.size() <= i)                // ensure enough space in the remainder, probably not needed
            r.a.resize(i+1, BezOrd(0,0));
        std::cout << "t1i=" << t1i << std::endl;
        BezOrd ci(r[i][0]/t1i[0], r[i][1]/t1i[1]); // c_i(v) := H_0(r_i(u)/(t_1)^i; u)
        std::cout << "ci=" << ci << std::endl;
        for(int dim = 0; dim < 2; dim++) // t1^i *= t1
            t1i[dim] *= t1[dim];
        c.a.push_back(ci);               // c(v) := c(v) + c_i(v)*t^i
        r = r - multiply(ci,ti);         // r(u) := r(u) - c_i(u)*(t(u))^i
        ti = multiply(ti,t);
        std::cout << "iteration\n";
    }
    
    return c;
}


void SBasis::normalize() {
    while(0 == a.back()[0] && 0 == a.back()[1])
        a.pop_back();
}

SBasis sin(double a0, double a1, int k) {
    SBasis s = BezOrd(sin(a0), sin(a1));
    Tri tr(s[0]);
    double t2 = (a1 - a0);
    s.a.push_back(BezOrd(cos(a0)*t2 - tr, -cos(a1)*t2 + tr));
    
    t2 *= t2;
    for(int i = 0; i < k; i++) {
        BezOrd bo(4*(i+1)*s[i+1][0] - 2*s[i+1][1],
                  -2*s[i+1][0] + 4*(i+1)*s[i+1][1]);
        bo += -(t2/(i+1))*s[i];
        
        
        s.a.push_back((1./(i+2))*bo);
    }
    
    return s;
}

SBasis cos(double a0, double a1, int k) {
    SBasis s = BezOrd(cos(a0), cos(a1));
    Tri tr(s[0]);
    double t2 = (a1 - a0);
    s.a.push_back(BezOrd(-sin(a0)*t2 - tr, sin(a1)*t2 + tr));
    
    t2 *= t2;
    for(int i = 0; i < k; i++) {
        BezOrd bo(4*(i+1)*s[i+1][0] - 2*s[i+1][1],
                  -2*s[i+1][0] + 4*(i+1)*s[i+1][1]);
        bo += -(t2/(i+1))*s[i];
        
        
        s.a.push_back((1./(i+2))*bo);
    }
    
    return s;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
