package br.ufscar.dc.ppgcc.gsdr.minas.kmeans

import grizzled.slf4j.Logger

import scala.annotation.tailrec
import scala.collection.immutable

import br.ufscar.dc.ppgcc.gsdr.minas.helpers.VectorStatistics

object Kmeans {
  val LOG = Logger(getClass)

  def closestCluster(point: Point, clusters: Seq[Cluster])(implicit distanceOperator: Point.DistanceOperator): (Point, Cluster, Double) = {
    clusters.map(c => (point, c, c.center.distance(point))).minBy(d => d._3)
  }

  def withFillerClusters(points: Vector[Point], clusters: Vector[Cluster], map: Map[Cluster, Vector[(Point, Double)]])
    (implicit distanceOperator: Point.DistanceOperator): Map[Cluster, Vector[(Point, Double)]] = {
    val keySet = map.keySet
    val missingClusters = clusters.filter(c => !keySet.contains(c))
    val missingClustersFiller: Iterable[(Cluster, Vector[(Point, Double)])] = if (missingClusters.nonEmpty) {
      // groupByClosest(points, missingClusters)
      missingClusters.map(c => c -> Vector(points.map(p => (p, p.distance(c.center))).minBy(d => d._2)))
    } else Map.empty[Cluster, Vector[(Point, Double)]]
    val exclusivePoints: Set[Long] = missingClustersFiller.flatMap(d => d._2.map(p => p._1.id)).toSet
    val finalMap = map.map(i => i._1 -> i._2.filterNot(p => exclusivePoints.contains(p._1.id))).++(missingClustersFiller)
    finalMap
  }

  def groupByClosest(points: Vector[Point], clusters: Vector[Cluster])(implicit distanceOperator: Point.DistanceOperator): Map[Cluster, Vector[(Point, Double)]] = {
    val map = points.map(p => clusters.map(c => (p, c, c.center.distance(p))).minBy(d => d._3))
      .groupBy(d => d._2).map(i => i._1 -> i._2.map(d => (d._1, d._3)))
    if (clusters.size != map.keySet.size) {
      LOG.error(s"No missing clusters are allowed. Expected ${clusters.size} got ${map.keySet.size}, ${clusters.map(p => p.id)} => ${map.keySet.map(p => p.id)}")
      val finalmap = withFillerClusters(points, clusters, map)
      if (clusters.size != finalmap.keySet.size) {
        LOG.error(s"AGGAIN No missing clusters are allowed. Expected ${clusters.size} got ${finalmap.keySet.size}, ${clusters.map(p => p.id)} => ${finalmap.keySet.map(p => p.id)}")
      }
      finalmap
    } else map
    // assert(clusters.size == map.keySet.size, s"No missing clusters are allowed. Expected ${clusters.size} got ${map.keySet.size}, $map")
    // map
  }

  def updateClustersVariance(clusterDistanceMap: Map[Cluster, Vector[(Point, Double)]]): Vector[Cluster] = {
    clusterDistanceMap.map(sameCluster => {
      val cluster = sameCluster._1
      val variance = sameCluster._2.map(p => p._2).max
      Cluster(cluster.id, cluster.center, variance, cluster.label)
    }).toVector
  }

  def updateClustersCenters(clusterDistanceMap: Map[Cluster, Vector[(Point, Double)]])(implicit distanceOperator: Point.DistanceOperator): Vector[(Cluster, Double)] = {
    clusterDistanceMap.map(sameCluster => {
      val cluster = sameCluster._1
      val points = sameCluster._2.map(p => p._1)
      val variance = sameCluster._2.map(p => p._2).max
      val center = points.reduce((a, b) => a + b) / points.size
      val movement = cluster.center.distance(center)
      // LOG.info(s"Cluster ${cluster.id} moved $movement => $center")
      // (Cluster(cluster.id, center, Some(points), Some(sameCluster._2), Some(variance)), movement)
      (Cluster(cluster.id, center, variance, cluster.label), movement)
    }).toVector
  }

  @tailrec
  def kmeans(label: String,
              points: Vector[Point], clusters: Vector[Cluster],
              prevMovement: Double = Point.max().fromOrigin, targetImprovement: Double = 10E-5,
              limit: Int = 10, i: Int = 0
  )(implicit distanceOperator: Point.DistanceOperator): Vector[Cluster] = {
    if (i > limit) clusters
    else {
      val distances = groupByClosest(points, clusters)
      if (distances.keys.size != clusters.size) {
        LOG.warn(s"kmeans [$label/$i] Reduction in clusters array. Had ${clusters.size} and got ${distances.keys.size}. i=$i, target=$targetImprovement")
      }
      val newClusters = updateClustersCenters(distances)
      val totalMovement = newClusters.map(c => c._2).sum
      val improvement = totalMovement / prevMovement
      val onlyNewClusters = newClusters.map(c => c._1)
      val innerDistances = VectorStatistics.VectorStdDev(onlyNewClusters.map(_.variance))
      LOG.info(s"kmeans [$label/$i] c=${clusters.size} p=${points.size} inner=$innerDistances / $limit, improvement=$improvement / $targetImprovement")
      if (improvement > 1.0) LOG.info(s"kmeans [$label/$i] worst result $prevMovement -> $totalMovement = $improvement / $targetImprovement")
      if (improvement < targetImprovement) onlyNewClusters
      else kmeans(label, points, onlyNewClusters, totalMovement, targetImprovement, limit, i + 1)
    }
  }

  def kmeansInitialRandom(label: String, k: Int, points: Vector[Point])(implicit distanceOperator: Point.DistanceOperator): Vector[Cluster] = {
    val sorted = points.sortBy(p => p.fromOrigin)
    val nPoints = sorted.size
    val step = nPoints / k
    // println(s"step => $step")
    val choosed: Vector[Int] = 0.until(nPoints, step).toVector.tail
    // println(s"choosed => $choosed")
    val clusters = choosed.map(i => {
      val p = sorted(i)
      Cluster(p.id, p, 0.0, label)
    })
    // println(s"clusters => ${clusters.size}")
    val result = updateClustersVariance(groupByClosest(points, clusters))
    // println(s"result => ${result.map(p => p.center.fromOrigin)}")
    result
  }
}
