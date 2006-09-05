#include <math.h>

#include "s-basis.h"

void bounds(SBasis const & s,
            double &lo, double &hi) {
    lo = std::min(s[0][0], s[0][1]);
    hi = std::max(s[0][0], s[0][1]);
    double ss = 0.25;
    for(unsigned i = 1; i < s.size(); i++) {
        double b = fabs(Hat(s[i]))*ss;
        lo -= b;
        hi += b;
        ss *= 0.25;
    }
}

#if 0
double Laguerre_internal_complex(SBasis const & p,
                                 double x0,
                                 double tol,
                                 bool & quad_root) {
    double a = 2*tol;
    double xk = x0;
    double n = p.degree();
    quad_root = false;
    while(std::norm(a) > (tol*tol)) {
        //std::cout << "xk = " << xk << std::endl;
        double b = p.back();
        double d = 0, f = 0;
        double err = abs(b);
        double abx = abs(xk);
        for(int j = p.size()-2; j >= 0; j--) {
            f = xk*f + d;
            d = xk*d + b;
            b = xk*b + p[j];
            err = abs(b) + abx*err;
        }
        
        err *= 1e-7; // magic epsilon for convergence, should be computed from tol
        
        double px = b;
        if(abs(b) < err)
            return xk;
        //if(std::norm(px) < tol*tol)
        //    return xk;
        double G = d / px;
        double H = G*G - f / px;
        
        //std::cout << "G = " << G << "H = " << H;
        double radicand = (n - 1)*(n*H-G*G);
        //assert(radicand.real() > 0);
        if(radicand < 0)
            quad_root = true;
        //std::cout << "radicand = " << radicand << std::endl;
        if(G.real() < 0) // here we try to maximise the denominator avoiding cancellation
            a = - sqrt(radicand);
        else
            a = sqrt(radicand);
        //std::cout << "a = " << a << std::endl;
        a = n / (a + G);
        //std::cout << "a = " << a << std::endl;
        xk -= a;
    }
    //std::cout << "xk = " << xk << std::endl;
    return xk;
}
#endif

void subdiv_sbasis(SBasis const & s,
                   std::vector<double> & roots, 
                   double left, double right) {
    double lo, hi;
    bounds(s, lo, hi);
    if(lo > 0 || hi < 0)
        return; // no roots here
    if(s.tail_error(1) < 1e-7) {
        double t = s[0][0] / (s[0][0] - s[0][1]);
        roots.push_back(left*(1-t) + t*right);
        return;
    }
    double middle = (left + right)/2;
    subdiv_sbasis(compose(s, BezOrd(0, 0.5)), roots, left, middle);
    subdiv_sbasis(compose(s, BezOrd(0.5, 1.)), roots, middle, right);
}

std::vector<double> roots(SBasis const & s) {
    std::vector<double> r;
    subdiv_sbasis(s, r, 0, 1);
    return r;
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
