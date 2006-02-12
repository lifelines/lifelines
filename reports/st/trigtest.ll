/*
 * @progname       trigtest.ll
 * @version        1.0
 * @author         Matt Emmerton
 * @category
 * @output         Text
 * @description    Tests functionality of trig-related functions.
 *
 * Test trig and spherical distance calculations
 *
 */

options("explicitvars")

proc main()
{
  print("Simple Sine/Cosine/Tangent operations (degree->value)", nl())
  print(nl())

  set(angle1,0)
  set(angle2,45)
  set(angle3a,89.99)
  set(angle3b,90)
  set(angle3c,90.01)
  set(angle4,135)
  set(angle5,180)
  set(angle6,225)
  set(angle7a,269.99)
  set(angle7b,270)
  set(angle7c,270.01)
  set(angle8,315)
  set(angle9,360)

  print("angle\tsin\tcos\ttan", nl())
  print(f(angle1),  "\t", f(sin(angle1)),  "\t", f(cos(angle1)),  "\t", f(tan(angle1)),  nl())
  print(f(angle2),  "\t", f(sin(angle2)),  "\t", f(cos(angle2)),  "\t", f(tan(angle2)),  nl())
  print(f(angle3a), "\t", f(sin(angle3a)), "\t", f(cos(angle3a)), "\t", f(tan(angle3a)), nl())
  print(f(angle3b), "\t", f(sin(angle3b)), "\t", f(cos(angle3b)), "\t", "N/A",           nl())
  print(f(angle3c), "\t", f(sin(angle3c)), "\t", f(cos(angle3c)), "\t", f(tan(angle3c)), nl())
  print(f(angle4),  "\t", f(sin(angle4)),  "\t", f(cos(angle4)),  "\t", f(tan(angle4)),  nl())
  print(f(angle5),  "\t", f(sin(angle5)),  "\t", f(cos(angle5)),  "\t", f(tan(angle5)),  nl())
  print(f(angle6),  "\t", f(sin(angle6)),  "\t", f(cos(angle6)),  "\t", f(tan(angle6)),  nl())
  print(f(angle7a), "\t", f(sin(angle7a)), "\t", f(cos(angle7a)), "\t", f(tan(angle7a)), nl())
  print(f(angle7b), "\t", f(sin(angle7b)), "\t", f(cos(angle7b)), "\t", "N/A",           nl())
  print(f(angle7c), "\t", f(sin(angle7c)), "\t", f(cos(angle7c)), "\t", f(tan(angle7c)), nl())
  print(f(angle8),  "\t", f(sin(angle8)),  "\t", f(cos(angle8)),  "\t", f(tan(angle8)),  nl())
  print(f(angle9),  "\t", f(sin(angle9)),  "\t", f(cos(angle9)),  "\t", f(tan(angle9)),  nl())
  print(nl())

  print("Simple ArcSine/ArcCosine/ArcTangent operations (value->degree)", nl())
  print(nl())

  set(value1,-1)
  set(value2,-0.707)
  set(value3,-0.3535)
  set(value4,0.0)
  set(value5,0.3535)
  set(value6,0.707)
  set(value7,1.0)

  print("value\tarcsin\tarccos\tarctan", nl())
  print(f(value1), "\t", f(arcsin(value1)), "\t", f(arccos(value1)), "\t", f(arctan(value1)), nl())
  print(f(value2), "\t", f(arcsin(value2)), "\t", f(arccos(value2)), "\t", f(arctan(value2)), nl())
  print(f(value3), "\t", f(arcsin(value3)), "\t", f(arccos(value3)), "\t", f(arctan(value3)), nl())
  print(f(value4), "\t", f(arcsin(value4)), "\t", f(arccos(value4)), "\t", f(arctan(value4)), nl())
  print(f(value5), "\t", f(arcsin(value5)), "\t", f(arccos(value5)), "\t", f(arctan(value5)), nl())
  print(f(value6), "\t", f(arcsin(value6)), "\t", f(arccos(value6)), "\t", f(arctan(value6)), nl())
  print(f(value7), "\t", f(arcsin(value7)), "\t", f(arccos(value7)), "\t", f(arctan(value7)), nl())
  print(nl())

  print("Reflexive operations (arcOP(OP(degree)) == degree)", nl())
  print("NOTE: Due to the periodic nature of these functions, output degree values may be",nl())
  print("different than the input degree values.", nl())
  print("NOTE: Due to roundoff, values may be out by a value of one in the least significant place.", nl())
  print(nl())

  print("angle\t\tarcsin(sin)\tarccos(cos)\tarctan(tan)", nl())
  print(f(angle1),   "\t\t", f(arcsin(sin(angle1))),   "\t\t", f(arccos(cos(angle1))),   "\t\t", f(arctan(tan(angle1))),   nl())
  print(f(angle2),   "\t\t", f(arcsin(sin(angle2))),   "\t\t", f(arccos(cos(angle2))),   "\t\t", f(arctan(tan(angle2))),   nl())
  print(f(angle3a),  "\t\t", f(arcsin(sin(angle3a))),  "\t\t", f(arccos(cos(angle3a))),  "\t\t", f(arctan(tan(angle3a))),  nl())
  print(f(angle3c),  "\t\t", f(arcsin(sin(angle3c))),  "\t\t", f(arccos(cos(angle3c))),  "\t\t", f(arctan(tan(angle3c))),  nl())
  print(f(angle4),   "\t\t", f(arcsin(sin(angle4))),   "\t\t", f(arccos(cos(angle4))),   "\t\t", f(arctan(tan(angle4))),   nl())
  print(f(angle5),   "\t\t", f(arcsin(sin(angle5))),   "\t\t", f(arccos(cos(angle5))),   "\t\t", f(arctan(tan(angle5))),   nl())
  print(f(angle6),   "\t\t", f(arcsin(sin(angle6))),   "\t\t", f(arccos(cos(angle6))),   "\t\t", f(arctan(tan(angle6))),   nl())
  print(f(angle7a),  "\t\t", f(arcsin(sin(angle7a))),  "\t\t", f(arccos(cos(angle7a))),  "\t\t", f(arctan(tan(angle7a))),  nl())
  print(f(angle7c),  "\t\t", f(arcsin(sin(angle7c))),  "\t\t", f(arccos(cos(angle7c))),  "\t\t", f(arctan(tan(angle7c))),  nl())
  print(f(angle8),   "\t\t", f(arcsin(sin(angle8))),   "\t\t", f(arccos(cos(angle8))),   "\t\t", f(arctan(tan(angle8))),   nl())
  print(f(angle9),   "\t\t", f(arcsin(sin(angle9))),   "\t\t", f(arccos(cos(angle9))),   "\t\t", f(arctan(tan(angle9))),   nl())
  print(nl())

  print("Decimal Degrees to DMH Conversions", nl())
  print(nl())

  set(deg1,44)
  set(min1,17)
  set(sec1,29)

  dms2deg(deg1,min1,sec1,dec1)
  print(d(deg1), " degrees, ", d(min1), " minutes and ", d(sec1), " seconds = ", f(dec1), " degrees.", nl())

  deg2dms(dec1,deg1,min1,sec1)
  print(f(dec1), " degrees = ", d(deg1), " degrees, ", d(min1), " minutes and ", d(sec1), " seconds.", nl())
  print(nl())

  print("Spherical Distance Calculations", nl())
  print(nl())

  /* 43.410815 / 43^24'38" is my house (lat) */
  set(deg1,43)
  set(min1,24)
  set(sec1,38)
  dms2deg(deg1,min1,sec1,dec1)

  /* -80.508982 / -80^30'32" is my house (lon) */
  set(deg2,-80)
  set(min2,30)
  set(sec2,32)
  dms2deg(deg2,min2,sec2,dec2)

  /* 44.101825 / 44^06'06" is my cottage (lat) */
  set(deg3,44)
  set(min3,06)
  set(sec3,06)
  dms2deg(deg3,min3,sec3,dec3)

  /* -81.721931 / -81^43'18" is my cottage (lon) */
  set(deg4,-81)
  set(min4,43)
  set(sec4,18)
  dms2deg(deg4,min4,sec4,dec4)

  print("House Lat: ", d(deg1), " degrees, ", d(min1), " minutes and ", d(sec1), " seconds = ", f(dec1), " degrees.", nl())
  print("House Lon: ", d(deg2), " degrees, ", d(min2), " minutes and ", d(sec2), " seconds = ", f(dec2), " degrees.", nl())
  print("Cottage Lat: ", d(deg3), " degrees, ", d(min3), " minutes and ", d(sec3), " seconds = ", f(dec3), " degrees.", nl())
  print("Cottage Lon: ", d(deg4), " degrees, ", d(min4), " minutes and ", d(sec4), " seconds = ", f(dec4), " degrees.", nl())

  print("House to Cottage: ", f(spdist(dec1,dec2,dec3,dec4)), nl())
  print("House to Cottage (via roads, suggested by Google Maps: ", f(138.2))
}
