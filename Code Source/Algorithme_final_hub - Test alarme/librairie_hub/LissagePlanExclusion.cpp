#include "LissagePlanExclusion.hpp"
#include <iostream>

#pragma GCC optimize ("O0")

namespace LissageVehicule {

    // --- FONCTION 1 : LE PLAN MOYEN (Régression Orthogonale via ACP) ---
    PlanLocal calculerPlanMoyen(const std::vector<V3>& points) {
        PlanLocal plan;
        int n = points.size();

        if (n < 3) {
            plan.centre = V3(0,0,0); plan.normale = V3(0,0,1); plan.axeU = V3(1,0,0); plan.axeV = V3(0,1,0);
            return plan;
        }

        Eigen::Vector3f centre(0, 0, 0);
        for (const auto& p : points) {
            centre.x() += p.getX();
            centre.y() += p.getY();
            centre.z() += p.getZ();
        }
        centre /= n;

        Eigen::Matrix3f covariance = Eigen::Matrix3f::Zero();
        for (const auto& p : points) {
            Eigen::Vector3f pt(p.getX() - centre.x(), p.getY() - centre.y(), p.getZ() - centre.z());
            covariance += pt * pt.transpose();
        }
        covariance /= (n - 1);

        Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> solver(covariance);
        Eigen::Vector3f normale_eigen = solver.eigenvectors().col(0).normalized();

        Eigen::Vector3f arbitraire(1, 0, 0);
        if (std::abs(normale_eigen.dot(arbitraire)) > 0.9f) { 
            arbitraire = Eigen::Vector3f(0, 1, 0);
        }
        
        Eigen::Vector3f axe_u = arbitraire.cross(normale_eigen).normalized();
        Eigen::Vector3f axe_v = normale_eigen.cross(axe_u).normalized();

        plan.centre = V3(centre.x(), centre.y(), centre.z());
        plan.normale = V3(normale_eigen.x(), normale_eigen.y(), normale_eigen.z());
        plan.axeU = V3(axe_u.x(), axe_u.y(), axe_u.z());
        plan.axeV = V3(axe_v.x(), axe_v.y(), axe_v.z());

        plan.a = plan.normale.getX(); plan.b = plan.normale.getY(); plan.c = plan.normale.getZ();
        plan.d = -(plan.a * plan.centre.getX() + plan.b * plan.centre.getY() + plan.c * plan.centre.getZ());

        return plan;
    }


    // --- FONCTION 2 : PROJECTION ET CRÉATION DU POLYGONE ---
    Polygone echantillonner64Points(int pId, const PlanLocal& plan, const std::vector<V3>& points3D) {
        std::vector<V3> pointsFinaux3D;

        if (points3D.empty()) return Polygone(pId, pointsFinaux3D);

        // 1. PROJECTION SUR LE NOUVEAU PLAN 2D
        struct Point2D { float x; float y; };
        std::vector<Point2D> pointsProjetes;

        Eigen::Vector3f c(plan.centre.getX(), plan.centre.getY(), plan.centre.getZ());
        Eigen::Vector3f u(plan.axeU.getX(), plan.axeU.getY(), plan.axeU.getZ());
        Eigen::Vector3f v(plan.axeV.getX(), plan.axeV.getY(), plan.axeV.getZ());

        for (const auto& p : points3D) {
            Eigen::Vector3f pt(p.getX() - c.x(), p.getY() - c.y(), p.getZ() - c.z());
            pointsProjetes.push_back({ pt.dot(u), pt.dot(v) });
        }
        pointsProjetes.push_back(pointsProjetes[0]);

        // 2. CALCUL DES DISTANCES ET DU PÉRIMÈTRE
        std::vector<float> distancesCumulees;
        distancesCumulees.push_back(0.0f);
        
        float perimetreTotal = 0.0f;
        for (size_t i = 0; i < pointsProjetes.size() - 1; ++i) {
            float dx = pointsProjetes[i+1].x - pointsProjetes[i].x;
            float dy = pointsProjetes[i+1].y - pointsProjetes[i].y;
            perimetreTotal += std::sqrt(dx*dx + dy*dy);
            distancesCumulees.push_back(perimetreTotal);
        }

        // 3. RÉÉCHANTILLONNAGE 
        float longueurStep = perimetreTotal / 63.0f; 
        
        // Le point 0
        V3 pt0_3D = plan.centre + (plan.axeU * pointsProjetes[0].x) + (plan.axeV * pointsProjetes[0].y);
        pointsFinaux3D.push_back(pt0_3D);

        int indexSegmentActuel = 0;

        // Les 62 points du milieu
        for (int i = 1; i < 63; ++i) {
            float distanceCible = i * longueurStep;

            while (indexSegmentActuel < pointsProjetes.size() - 1 && 
                   distancesCumulees[indexSegmentActuel + 1] < distanceCible) {
                indexSegmentActuel++;
            }

            float distDansSegment = distanceCible - distancesCumulees[indexSegmentActuel];
            float longSegment = distancesCumulees[indexSegmentActuel + 1] - distancesCumulees[indexSegmentActuel];
            float ratio = (longSegment > 0.0001f) ? (distDansSegment / longSegment) : 0.0f;

            // Coordonnées 2D interpolées
            float interX = pointsProjetes[indexSegmentActuel].x + ratio * (pointsProjetes[indexSegmentActuel+1].x - pointsProjetes[indexSegmentActuel].x);
            float interY = pointsProjetes[indexSegmentActuel].y + ratio * (pointsProjetes[indexSegmentActuel+1].y - pointsProjetes[indexSegmentActuel].y);
            
            // Re-transformation en V3 !
            V3 ptInterpole3D = plan.centre + (plan.axeU * interX) + (plan.axeV * interY);
            pointsFinaux3D.push_back(ptInterpole3D);
        }

        // Le 64ème point (index 63)
        pointsFinaux3D.push_back(pointsFinaux3D[0]);

        // 4. On renvoie directement l'objet Polygone 
        return Polygone(pId, pointsFinaux3D);
    }
}